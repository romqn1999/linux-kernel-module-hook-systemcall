#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int main(){
	int fd = open("text.txt", O_WRONLY | O_CREAT);
	
	printf("fd = %d\n", fd);

	char s[] = "This will be output to text.txt\n";
	write(fd, s, strlen(s));

	close(fd);
	return 0;
}
