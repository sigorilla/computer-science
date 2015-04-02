#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

const int STRING_SIZE = 30;
const int AMOUNT_OF_PARAMETERS = 5;

int MAX_TIME = 0, file_sec, status, i = 0, ret_read = 0, 
ret_execvp = 0, count_sym = 0, count_word = 0, fd, count_time;
pid_t CPID_MAX, pid, ret_waitpid;
char ** file_line, file_sym, * path;

void free_mem();

int main( int argc, char ** argv ) {

	path = "default.txt";
	if ( argc == 2 ) {
		path = argv[ 1 ];
	}

	fd = open( path, O_RDONLY );
	if ( fd == -1 ) {
		printf( "Error opening file: %s\n", strerror( errno ) );
		goto out;
	} else {
		printf( "File: %s\n", path );
	}

	// do while read doesn't return 0 
	do {
		file_line = ( char ** ) malloc( sizeof( char * ) * AMOUNT_OF_PARAMETERS );
		if ( file_line == NULL ) {
			printf( "Error of allocating memory\n" );
			goto out;
		}

		// memory allocating 
		for ( i = 0; i < AMOUNT_OF_PARAMETERS; i++ ) {
			file_line[ i ] = ( char * ) malloc( sizeof( char ) * STRING_SIZE );
			if ( file_line[ i ] == NULL ) {
				printf( "Error of allocating memory\n" );
				goto out;
			}
		}
		count_word = 0;
		count_sym = 0;

		// reading line with time, name of program, parameters while return read is not equal 0 
		while ( (ret_read = read( fd, &file_sym, 1 ) != 0) && (file_sym != '\n') ) {

			// checking error 
			if ( ret_read == -1 ) {
				printf( "Error of file reading: %s\n", strerror( errno ) );
				goto out;
			}

			// read every sign into file_line 
			if ( file_sym != ' ' ) {
				file_line[ count_word ][ count_sym ] = file_sym;
				count_sym++;
			} else {
				// every string of parameters ends by '\0'
				file_line[ count_word ][ count_sym ] = '\0';
				count_sym = 0;
				count_word++;
			}
		}
		
		if ( file_sym == '\n' ) {
			file_line[ count_word ][ count_sym ] = '\0';
		}

		if ( ret_read != 0 ) {
			// converting string to time 
			file_sec = strtol( file_line[ 0 ], NULL, 10 );

			// for right working execvp last element of array is NULL 
			file_line[ count_word + 1 ] = NULL;

			// create a child process 
			pid = fork();
			if ( pid == -1 ) {
				printf( "Error of creating process: %s\n", strerror( errno ) );
				goto out;
			} else if ( pid == 0 ) {
				// pid = 0 => it's a child process 
				// time of sleeping of child process 
				sleep( file_sec );

				ret_execvp = execvp( file_line[ 1 ], &file_line[ 1 ] );

				if ( ret_execvp == -1 ) {
					printf( "Error of launching process: %s\n", strerror( errno ) );
					goto out;
				}
			} else if ( file_sec > MAX_TIME ) {
				// parent wait his children 
				CPID_MAX = pid;
				MAX_TIME = file_sec;
			}
		}
	} while ( ret_read != 0 );

	ret_waitpid = waitpid( CPID_MAX, NULL, 0 );

	// checking errors in waitpid 
	if ( ret_waitpid == -1 ) {
		printf( "Error of waiting child process: %s\n", strerror( errno ) );
		goto out;
	}

	out:
		free_mem();
		return 0;
}

void free_mem() {
	for ( i = 0; i <= count_word; i++ ) {
		free( file_line[ i ] );
	}
	free( file_line );
}
