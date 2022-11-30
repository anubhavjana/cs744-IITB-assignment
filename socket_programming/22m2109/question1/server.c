#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<signal.h>

#include <netinet/in.h>

#include <pthread.h>

void * handle_connection(void* clientfd); //each thread for each client


void error(char *msg) {
  perror(msg);
  //exit(1);
  return;
}


int main(int argc, char *argv[]) 
{
  
  int i;
  
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

    /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
    */

    bzero((char *)&serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
      error("ERROR on binding");

  /* listen for incoming connection requests */

    listen(sockfd,50);
    clilen = sizeof(cli_addr);

 
    while(1)
    {
        //signal(SIGPIPE,SIG_IGN);
        printf("Waiting for new connecions...\n");
        /* accept a new request, create a newsockfd */

        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        error("ERROR on accept");
        
        pthread_t t;
        int *pclient = malloc(sizeof(int));
        *pclient = newsockfd;
        pthread_create(&t,NULL,handle_connection,pclient);
    }
    return 0;

}


void *handle_connection(void* clientfd)
{
    
    int clientsocket = *((int*)clientfd);
    free(clientfd);
    int n;
    char buffer[256];

    /* read message from client */
    
    
    while(1)
    {
    
    memset(buffer,0,256); // zero out the receive buffer to make sure it has null terminated
    n = read(clientsocket, buffer, 255);
    //printf("Read return n value = %d\n",n);
    if(n==0 || n==-1)
    {
      bzero(buffer, 256);
      return NULL;
    }
    
    if (n < 0)
    {
     
      //pthread_exit(NULL);
      
      error("ERROR reading from socket");
    }
    printf("The thread id %d from client returned the message from server: %s",pthread_self(),  buffer);

    /* send reply to client */

    n = write(clientsocket, "I got your message", 18);
    
    //printf("Write return n value = %d\n",n);
    if (n < 0)
    {
   
      bzero(buffer, 256);
      pthread_exit(NULL);
      error("ERROR writing to socket");
    }

   
  }

  return NULL;
}

