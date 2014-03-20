/**
 * Simulator office as producer-consumer
 * USAGE: ./thread n k t N T L
 * n: numbers of clerks 
 * k: numbers of documents 
 * t: time to ready document 
 * N: numbers of scanners
 * T: time to scanning
 * L: length of conveer
 *
 * by Stepanov Igor 
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

struct _data {
	int idClerk;
	int idDocument;
} *conveer;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t conveerNotFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t conveerNotEmpty = PTHREAD_COND_INITIALIZER;
int i, n, k, t, N, T, L, all, *idsClerk, *idsScanner, 
	counter, ready, start, quit, clerkDone;

/* For debbuging */
void PrintArray() 
{
	for (i=0; i<L;i++)
		printf("%d >> |%d : %d|\t", i, conveer[i].idClerk, conveer[i].idDocument);
	printf("\n");
}

/* Producer */
void *Clerk (void* arg)
{
	int currentClerk = *(int*)arg, currentDocument = 0;
	++currentClerk;
	
	while (currentDocument < k) {
		sleep(t);
		pthread_mutex_lock(&mutex);
		while (ready == 1) {
			pthread_cond_wait(&conveerNotFull, &mutex);
			continue;
		}
		currentDocument++;
		/* Starting from 0 pointer */
		counter += start;
		start = 1;
		if (counter == L)
			counter = 0;
		conveer[counter].idClerk = currentClerk;
		conveer[counter].idDocument = currentDocument;
		ready++;
		clerkDone++;
		pthread_cond_signal(&conveerNotEmpty);
		pthread_mutex_unlock(&mutex);
		// PrintArray();
	}

	pthread_exit(NULL);
}

/* Consumer */
void *Scanner (void* arg)
{
	int currentClerk, currentDocument, currentScanner = *(int*)arg;
	++currentScanner;
	
	while (quit < all) {
		sleep(T);
		pthread_mutex_lock(&mutex);
		while (ready == 0 && quit < all) {
			pthread_cond_wait(&conveerNotEmpty, &mutex);
			continue;
		}
		/* Unlock mutex for waiting scanners */
		if (quit >= all) {
			pthread_cond_signal(&conveerNotEmpty);
			pthread_mutex_unlock(&mutex);
			break;
		}
		ready--;
		currentClerk = conveer[counter].idClerk;
		currentDocument = conveer[counter].idDocument;
		conveer[counter].idClerk = 0;
		conveer[counter].idDocument = 0;
		/* Increse counter when all clerks done */
		if (clerkDone >= all) {
			counter++;
			if (counter == L)
				counter = 0;
		}
		quit++;
		pthread_cond_signal(&conveerNotFull);
		pthread_mutex_unlock(&mutex);
		/* Print - id Clerk : id Document */
		printf("  #%d\t|   #%d\t >>   #%d\n", currentScanner, currentClerk, currentDocument);
	}
	
	pthread_exit(NULL);
}

int main(int argc, char** argv)
{
	if ( argc != 7 ) {
		printf("Wrong numbers of parameters. USAGE: n k t N T L\n");
		exit(-1);
	}
	
	//double start = time(NULL);
	
	n = atoi(argv[1]); 	// numbers of clerks 
	k = atoi(argv[2]); 	// numbers of documents 
	t = atoi(argv[3]); 	// time to ready document 
	N = atoi(argv[4]); 	// numbers of scanners
	T = atoi(argv[5]); 	// time to scanning
	L = atoi(argv[6]); 	// length of conveer
	all = n*k;			// numbers of all documents
	
	// if ( all <= N ) N = all;

	if ( !(conveer = (struct _data *)malloc(sizeof(struct _data)*L)) ) {
		printf("Error of allocating memory\n");
		exit(-1);
	}

	/* Array of clerk's and scanner's ID */
	if ( !(idsClerk = (int *)malloc(sizeof(int)*n)) ) {
		printf("Error of allocating memory\n");
		exit(-1);
	}
	if ( !(idsScanner = (int *)malloc(sizeof(int)*N)) ) {
		printf("Error of allocating memory\n");
		exit(-1);
	}

	printf(" IDsc\t|  IDcl\t >>  IDdoc\n");
	pthread_t tidClerk[n], tidScanner[N];
		
	for (i = 0; i < n; i++) {
		idsClerk[i] = i;
		if ( pthread_create(&tidClerk[i], NULL, Clerk, (void*)&idsClerk[i]) ) {
			printf("Error of creating thread: %s\n", strerror(errno));
			exit(-1);
		}
		/* Sleep for waiting transfer ID of Clerk */
		usleep(1);
	}
	
	for (i = 0; i < N; i++) {
		idsScanner[i] = i;
		if ( pthread_create(&tidScanner[i], NULL, Scanner, (void*)&idsScanner[i]) ) {
			printf("Error of creating thread: %s\n", strerror(errno));
			exit(-1);
		}
		/* Sleep for waiting transfer ID of Scanner */
		usleep(1);
	}
	
	for (i = 0; i < n; i++) 
		if ( pthread_join(tidClerk[i], NULL) ) {
			printf("Error of joining thread: %s\n", strerror(errno));
			exit(-1);
		}

	for (i = 0; i < N; i++) 
		if ( pthread_join(tidScanner[i], NULL) ) {
			printf("Error of joining thread: %s\n", strerror(errno));
			exit(-1);
		}
	
	//printf("It takes %.4lf sec.\n", (time(NULL) - start) );
		
	return 0;
		
}
