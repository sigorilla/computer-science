#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main( int argc, char ** agrv)
{
	int i, fd;
	if ((fd = open("1M", O_CREAT | O_WRONLY | O_TRUNC)) < 0 ){
	  perror("Can't open file ");
	  exit(EXIT_FAILURE);
	}
	for ( i=0; i<1048576; i++)
	{
		write(fd, "1", 1);
	}
	close(fd);
		
	return 0;
}

