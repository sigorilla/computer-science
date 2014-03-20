#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MAX_INPUT_LENGTH 255
#define STRING_SIZE 124
#define AMOUNT_OF_PARAMETERS 10
#define MAX_CONV 2

int i=0, j=0, ret_read = 0, ret_execvp = 0, count_word = 0, count[3], COUNT_PROGS = 1;
pid_t pidA, pidB, ret_waitpid;
size_t size;
char buff[MAX_INPUT_LENGTH], *** file_line, * str, word[1], prev[1];

void raw_input(char *prompt, char *buffer, size_t length)
{
	printf("%s", prompt);
	fgets(buffer, length, stdin);
}

int parser(char *string)
{
	char *token, *last;
	int symb, count_new = 0;
	
	count_word = 0;
	COUNT_PROGS = 1;
	token = strtok_r(string, " \n", &last);
	while (token != NULL) 
	{
		symb = token[0];
		if ( symb == 124  )
		{
			if ( strlen(token) == 1 )
			{
				file_line[COUNT_PROGS-1][count_new] = NULL;
				COUNT_PROGS = 2;
				count_new = 0;
				token = strtok_r(NULL, " \n", &last);
				continue;
			}
			else 
			{
				printf("If you want conveyor, you should write only one '|' symbol!\n");
				return -1;
			}
		}
		
		file_line[COUNT_PROGS-1][count_new] = token;
		count_new++;
		token = strtok_r(NULL, " \n", &last);
	}
	file_line[COUNT_PROGS-1][count_new] = NULL;
	
	return 0;
}

int main( int argc, char **argv)
{
	int fdA[2], fdB[2];
	str = (char *)malloc(sizeof(char)*MAX_INPUT_LENGTH);
	if ( str == NULL ) 
	{
		printf("Error of allocating memory\n");
		return -1;
	}
	
	printf(">>\t Hello user! \n");
	printf(">> Enter 'exit' to stop the shell.\n");

	while (1)
	{
		/* Wait command */
		raw_input(">> ", buff, sizeof buff);
		
		str = buff;
		if ( strcmp(str, "exit\n") == 0 )
		{
			break;
		}
		if ( strcmp(str, "\n") == 0 )
		{
			continue;
		}
		
		/* Allocate memory */
		file_line = (char ***)malloc(sizeof(char **)*COUNT_PROGS);
		if ( file_line == NULL ) 
		{
			printf("Error of allocating memory\n");
			goto out;
		}
		for (j=0; j<MAX_CONV; j++)
		{
			file_line[j] = (char **)malloc(sizeof(char *)*AMOUNT_OF_PARAMETERS);
			if ( file_line[j] == NULL ) 
			{
				printf("Error of allocating memory\n");
				goto out;
			}
			for (i=0; i<AMOUNT_OF_PARAMETERS; i++)
			{
				file_line[j][i] = (char *)malloc(sizeof(char)*STRING_SIZE);
				if (file_line[j][i] == NULL) 
				{
					printf("Error of allocating memory\n");
					goto out;
				}
			}
		}
		
		for (i=0; i<3; i++)
		{
			count[i] = 0;
		}
		
		/* Parsing input string */
		if ( parser(str) == -1 )
		{
			continue;
		}

		/* Create pipes */
		if( pipe(fdA) < 0 || pipe(fdB) < 0 ){
			printf("Can\'t create pipe\n");
			goto out;
		} 
		
	if ( COUNT_PROGS == 2 )
	{
				
		pidA = fork();
		if (pidA == -1) 
		{
			printf("Error of creating process A: %s\n", strerror(errno));
			goto out;
		}
		/* It's a parent process A */
		else if (pidA > 0)
		{			
			dup2(STDOUT_FILENO, fdA[1]);
			close(fdA[1]);
		}
		/* It's a child process A */
		else if (pidA == 0)
		{
			close(fdA[0]);
			dup2(fdA[1], STDOUT_FILENO);
			
			ret_execvp = execvp(file_line[0][0], &file_line[0][0]);

			if (ret_execvp == -1) 
			{
				printf("Error of launching process A: %s\n", strerror(errno));
				exit(0);
			}

			close(fdA[1]);
		}
		
		ret_waitpid = waitpid(pidA, NULL, 0);
		if (ret_waitpid == -1)
		{
			printf("Error of waiting child process A: %s\n", strerror(errno));
			exit(0);
		}
			
		pidB = fork();
		if (pidB == -1) 
		{
			printf("Error of creating process B: %s\n", strerror(errno));
			goto out;
		}
		/* It's a parent process B */
		else if (pidB > 0)
		{
			dup2(STDOUT_FILENO, fdB[1]);
			close(fdB[1]);
			
			*word = '\0';
			*prev = '\0';
			
			/* Print */
			do
			{
				size = read(fdB[0], word, 1);
				if ( size == -1)
				{
					printf("Error to read from file description: %s\n", strerror(errno));
					exit(0);
				}					
				printf("%s", word);
				count[0]++;
				if ( strcmp(word, " ") == 0  && strcmp(prev, " ") != 0 )
				{
					count[1]++;
				}
				if ( strcmp(word, "\n") == 0 )
				{
					count[2]++;
					if ( strcmp(prev, "\n") != 0)
					{
						count[1]++;
					}
				}
				prev[0] = word[0];
			} while( size != 0 );
			
			count[2]--;
			count[0]--;
			
			printf("----------------------------\n");
			printf("\t%d \t%d \t%d\n", count[2], count[1], count[0]);
			close(fdB[0]);
			
		}
		/* It's a child process B */
		else if (pidB == 0)
		{
			close(fdA[1]);
			close(fdB[0]);
			dup2(fdA[0], STDIN_FILENO);
			dup2(fdB[1], STDOUT_FILENO);
			
			ret_execvp = execvp(file_line[1][0], &file_line[1][0]);

			if (ret_execvp == -1) 
			{
				printf("Error of launching process B: %s\n", strerror(errno));
				exit(0);
			}

			close(fdA[0]);
			close(fdB[1]);
		}
			
		ret_waitpid = waitpid(pidB, NULL, 0);
		if (ret_waitpid == -1)
		{
			printf("Error of waiting child process B: %s\n", strerror(errno));
			exit(0);
		}
		
	} else {
		
		pidA = fork();
		if (pidA == -1) 
		{
			printf("Error of creating process: %s\n", strerror(errno));
			goto out;
		}
		/* It's a parent process A */
		else if (pidA > 0)
		{
			dup2(STDOUT_FILENO, fdA[1]);
			close(fdA[1]);
			
			*word = '\0';
			*prev = '\0';
			
			/* Print */
			do
			{
				size = read(fdA[0], word, 1);
				if ( size == -1)
				{
					printf("Error to read from file description: %s\n", strerror(errno));
					close(fdA[0]);
					exit(0);
				}					
				printf("%s", word);
				count[0]++;
				if ( strcmp(word, " ") == 0  && strcmp(prev, " ") != 0 )
				{
					count[1]++;
				}
				if ( strcmp(word, "\n") == 0 )
				{
					count[2]++;
					if ( strcmp(prev, "\n") != 0)
					{
						count[1]++;
					}
				}
				prev[0] = word[0];
			} while( size != 0 );
			
			count[2]--;
			count[0]--;
			
			printf("----------------------------\n");
			printf("\t%d \t%d \t%d\n", count[2], count[1], count[0]);
			
			close(fdA[0]);
		}
		/* It's a child process A */
		else if (pidA == 0)
		{
			close(fdA[0]);
			dup2(fdA[1], STDOUT_FILENO);
			
			ret_execvp = execvp(file_line[0][0], &file_line[0][0]);

			if (ret_execvp == -1) 
			{
				printf("Error of launching process: %s\n", strerror(errno));
				exit(0);
			}

			close(fdA[1]);
		}
		
		ret_waitpid = waitpid(pidA, NULL, 0);
		if (ret_waitpid == -1)
		{
			printf("Error of waiting child process A: %s\n", strerror(errno));
			exit(0);
		}
		
	}
			
		/* TODO: 
		 * free of memory
		 */
	}
	
	out:
	// Free of memory
	//free(str);
	/*for (j=0; j<MAX_CONV; j++)
	{
		for (i=0; i<AMOUNT_OF_PARAMETERS; i++)
		{			
			free(file_line[j][i]);
		}	
		free(file_line[j]);
	}	
	free(file_line);
	*/
	printf(">>\t Goodbuy!\n");
	printf(">>\t Have a nice day!\n");
	printf("\n");
	
	return 0;
}
