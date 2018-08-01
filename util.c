/*
 * util.c
 * variouse utility methods
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "util.h"


uint32_t string_to_ip(char *ipstr){
	uint32_t ip;
	uint8_t a, b, c, d;

	sscanf(ipstr, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d);
	ip = d << 24 | c << 16 | b << 8 | a;

	return ip;
}

char* ip_to_string(uint32_t ip){
	static char ipstr[16];

	snprintf(ipstr, 16, "%hhu.%hhu.%hhu.%hhu", ip & 0xff, (ip & 0xff00) >> 8,
			(ip & 0xff0000) >> 16, (ip & 0xff000000) >> 24);
	
	return ipstr;
}

int
view_media_from_binary(const char *filename, void *media_buf, int media_size)
{
	/*
	 * Takes a filename to write the image binary data to, 
	 * a pointer to the image binary data, and integer 
	 * describing the size of binary data.
	 * Returns 0 upon sucess, otherwise -1.
	 */
	 int fd;
	int pid, status;
	
	//open the file we will write the media data to
	if((fd = open(filename, O_CREAT | O_WRONLY, 0666)) < 0){
		perror("view_media_from_binary - open error:");
		return -1;
	}

	//write the media data
	if(write(fd, media_buf, media_size) != media_size){
		perror("view_media_from_binary - write error:");
		return -1;
	}
	
	if((pid = fork()) < 0){
		perror("view_media_from_binary - fork error:");
		return -1;
	}

	//exec - open the image
	if(pid == 0){
		execlp("xdg-open", "xdg-open", filename, (char *)0);
		exit(1);
	}
	else{
		if(waitpid(pid, &status, 0) < -1){
			printf("view_media_from_binary - waitpid error;\n");
			return -1;
		}
		return 0;
	}
}




