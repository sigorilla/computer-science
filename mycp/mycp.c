/**
 * My copy programm
 * USAGE: ./mycp [parametrs -l] name_from name_to
 * -l : copy as link
 * by Stepanov Igor 
 */
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define SIZE 1024

int check_arg (int argc, char **argv)
{
	if ( argc<3 || argc>4 ) 
	{
		printf("Wrong numbers of arguments.\n");
		return -1;
	}

	if ( argc==4 && strcmp(argv[1], "-l")!=0 )
	{
		printf("Writee '-l' as option. Or writee empty option.\n");
		return -1;
	}

	if ( argc==3 && strcmp(argv[1], "-l")==0 )
	{
		printf("Wrong arguments!\n");
		return -1;
	}

	return 1;
}

int main (int argc, char **argv)
{
	struct stat info;
	int fd_from, fd_to, error, part_from, part_to;
	char buffer[SIZE];
	char *readd, *writee;

	readd	= argv[argc-2];
	writee	= argv[argc-1];

	if ( check_arg(argc, argv)!=-1 )
	{
		if ( strcmp(argv[1], "-l")==0 )
		{
			if ( lstat(readd, &info)==-1 )
			{
				printf("Failed get link of %s : %s\n", readd, strerror(errno));
				return -1;
			}
			else if ( (info.st_mode & S_IFMT)==S_IFLNK )
			{
				if ( symlink(readd, writee)==-1 )
				{

					printf("Failed make a link %s: %s\n", writee, strerror(errno));
					goto out;
					return -1;
				}
				printf("Link copy.\n");
				return 0;
			}
		}

		fd_from = open(readd, O_RDONLY | O_NOFOLLOW);
		if ( fd_from==-1 )
		{
			goto out;
		}
		fstat(fd_from, &info);

		fd_to = open(writee, O_CREAT | O_WRONLY | O_EXCL);
		if ( fd_from==-1 )
		{
			goto out;
		}
		fchmod(fd_to, info.st_mode);

		do
		{
			part_from = read(fd_from, buffer, SIZE);
			part_to = write(fd_to, buffer, part_from);
		}
		while ( part_from>0 );

		if (part_to<0)
		{
			printf("Error writee: %s\n", strerror(errno));
			goto out;
			return -1;
		}

		out: 
		if (fd_from!=-1) 
			close(fd_from);
		if (fd_to!=-1) 
			close(fd_to);
			
	}

	return 0;
}
