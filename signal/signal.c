#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

int outChar = 0, byte = 128, i;
pid_t pid;

void childDead( int );
void parentDead( int );
void one( int );
void zero( int );
void empty( int );

int main( int argc, char ** argv ) {
	int fdFrom = 0, fdTo = 0, ret_waitpid;
	double start = clock();

	if ( argc != 3 ) {
		printf( "Error of number arguments (%d): %s\n", argc, strerror( errno ) );
		goto out;
	}

	pid_t ppid = getpid();

	sigset_t set;

	// Edit blocking of signal
	// If child is dead, then parent get SIGCHLD
	// struct sigaction act_exit;
	struct sigaction act_exit;
	memset( &act_exit, 0, sizeof( act_exit ) );
	act_exit.sa_handler = childDead;
	sigfillset( &act_exit.sa_mask );
	sigaction( SIGCHLD, &act_exit, NULL );

	// SIGUSR1 = 1
	struct sigaction act_one;
	memset( &act_one, 0, sizeof( act_one ) );
	act_one.sa_handler = one;
	sigfillset( &act_one.sa_mask );
	sigaction( SIGUSR1, &act_one, NULL );

	// SIGUSR2 = 0
	struct sigaction act_zero;
	memset( &act_zero, 0, sizeof( act_zero ) );
	act_zero.sa_handler = zero;
	sigfillset( &act_zero.sa_mask );
	sigaction( SIGUSR2, &act_zero, NULL );

	// Blocking
	sigaddset( &set, SIGUSR1 );
	sigaddset( &set, SIGUSR2 );
	sigaddset( &set, SIGCHLD );
	sigprocmask( SIG_BLOCK, &set, NULL );
	sigemptyset( &set );

	//Fork
	if ( (pid = fork()) == -1 ) {
		printf("Error of creating process: %s\n", strerror(errno));
		goto out;
	} else if (pid == 0) {
		// Child 
		char c = 0;

		sigemptyset( &set );

		// If parent is dead, then child get SIGALRM
		struct sigaction act_alarm;
		memset( &act_alarm, 0, sizeof( act_alarm ) );
		act_alarm.sa_handler = parentDead;
		sigfillset( &act_alarm.sa_mask );
		sigaction( SIGALRM, &act_alarm, NULL );

		// SIGUSR1 = nothing
		struct sigaction act_empty;
		memset( &act_empty, 0, sizeof( act_empty ) );
		act_empty.sa_handler = empty;
		sigfillset( &act_empty.sa_mask );
		sigaction( SIGUSR1, &act_empty, NULL );

		if ( (fdFrom = open( argv[ 1 ], O_RDONLY )) < 0 ) {
			printf( "Error of opening file %s: %s\n", argv[ 1 ], strerror( errno ) );
			goto out;
		}

		while ( read( fdFrom, &c, 1 ) > 0 ) {
			// Get SIGALRM if parent doesn't answer from 1 second
			alarm( 1 );

			for ( i = 128; i >= 1; i /= 2 ) {
				if ( i&c ) {
					kill( ppid, SIGUSR1 );
				} else {
					kill( ppid, SIGUSR2 );
				}
				// Wait check
				sigsuspend( &set );
			}
		}
		goto out;

	}
	errno = 0;
	/*ret_waitpid = waitpid(pid, NULL, 0);
	if (ret_waitpid == -1)
	{
		printf("Error of waiting child process: %s\n", strerror(errno));
		goto out;
	}*/

	if ( (fdTo = open( argv[ 2 ], O_CREAT | O_WRONLY | O_TRUNC )) < 0 ) {
		printf( "Error of opening file %s: %s\n", argv[ 2 ], strerror( errno ) );
		goto out;
	}

	do {
		// Whole byte
		if ( byte == 0 ) {
			write( fdTo, &outChar, 1 );
			byte = 128;
			outChar = 0;
		}
		sigsuspend(&set);
	} while ( 1 );
	// Break when child dead (get SIGCHLD) 

	out:
		if ( fdFrom != -1 ) {
			close( fdFrom );
		}
		if ( fdTo != -1 ) {
			close( fdTo );
		}

	/*
	 * File in 1M = 1048576 b
	 * bash:
	 * dd of=file bs=1 count=0 seek=1M
	 * time:
	 * ~102.72 seconds
	 * */

	printf( "It takes %.4lf sec.\n", ( clock() - start ) / CLOCKS_PER_SEC );

}

void childDead( int sig ) {
	exit( EXIT_SUCCESS );
}

void parentDead( int sig ) {
	exit( EXIT_SUCCESS );
}

void one( int sig ) {
	outChar += byte;
	byte /= 2;
	kill( pid, SIGUSR1 );
}

void zero( int sig ) {
	byte /= 2;
	kill( pid, SIGUSR1 );
}

void empty( int sig ) {
}
