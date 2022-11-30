#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];
int main(int argc, char *argv[])
{
    int fd = STDIN_FILENO;
    if(argc>1)
    {
        fd=open(argv[1],O_RDONLY);
        if(fd==-1)
        {
            write(STDERR_FILENO,"no file\n",8);
            exit(EXIT_FAILURE);

        }
    }
    ssize_t bytes_read;
    while((bytes_read = read(fd,buffer,BUFFER_SIZE)))//read 80 bytes from standard input --> file descriptor 0
    {
         write(STDOUT_FILENO,buffer,bytes_read); //write only those byte read
    } 
    close(fd);
    return 0;

}