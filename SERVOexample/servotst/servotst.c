//Laurien Santin e Luis Miguel Batista

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc,char *argv[])
{
	int fd;
	
	if(argc != 3)
	{
		fprintf(stderr,"servotst\n");
		fprintf(stderr,"\tUsage:\t%s <device> <value>\n",argv[0]);
		return -1;
	}
	if((fd=open(argv[1],O_WRONLY))==-1)
	{
		perror("servotst");
		return -errno;
	}
	
	if(write(fd,&argv[2][0],1)==-1)
	{
		perror("servotst");
		return -errno;
	}
	
	sleep(5);
	
	if(close(fd))
	{
		perror("servotst");
		return -errno;
	}
	
	return 0;
}
