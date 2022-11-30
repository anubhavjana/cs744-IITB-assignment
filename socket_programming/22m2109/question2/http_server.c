#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<signal.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <pthread.h>


typedef struct HTTP_Response {
  char *HTTP_version; // 1.0 for this assignment

  char *status_code; // ex: 200, 404, etc.
  char *content_type; // ex: text/html  etc.
  int  *content_length;
}http_response;

void * handle_connection(void* clientfd); //each thread for each client

void error(char *msg) {
  perror(msg);
  exit(1);
  //return;
}

// reads text from path to source 
void read_file (char path[], char **source) {
    FILE *f = fopen(path, "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    *source = malloc(fsize + 1);
    fread(*source, fsize, 1, f);
    fclose(f);

    // *source[fsize]='\0'; // this line causes a segfault
}

void concatenate_string(char* s, char* s1)
{
    int i;
  
    int j = strlen(s);
  
    for (i = 0; s1[i] != '\0'; i++) {
        s[i + j] = s1[i];
    }
  
    s[i + j] = '\0';
  
    return;
}


int main(int argc, char *argv[]) 
{

    char url_tokens[50][50]; 
    int i, j, cnt;
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr,cli_addr;
    
    if (argc < 2)
    {
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
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    

    /* bind socket to this port number on this machine */

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    /* listen for incoming connection requests */

    
    int n;
    char buffer[256];
    while(1)
    {
      printf("Waiting for new connecions...\n");
      listen(sockfd,5);
      socklen_t clilen;
      clilen = sizeof(cli_addr); 
      
      
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
    
  char url_tokens[50][50]; 
  int i=0;
  int clientsocket = *((int*)clientfd);
  free(clientfd);
  int n;
  
    char buffer[256];
    
    memset(buffer,0,256); // zero out the receive buffer to make sure it has null terminated
    n = read(clientsocket, buffer, 255);

    int j = 0;
    int cnt = 0;
    for (i = 0; i <= (strlen(buffer)); i++)
    {
      // if space or NULL found, assign NULL into splitStrings[cnt]
        if (buffer[i] == ' ' || buffer[i] == '\0') 
        {
            url_tokens[cnt][j] = '\0';
            cnt++; //for next word
            j = 0; //for next word, init index to 0
        }
        else
        {
            url_tokens[cnt][j] = buffer[i];
            j++;
        }
    }
    //printf("Buffer is : %s\n",buffer);
    //printf("URL is : %s\n",url_tokens[1]+1);
    if(strcmp(url_tokens[0],"GET")!=0)
    {
      printf("Method not supported\n");
      exit(0);
    }
    struct stat buffer1;
    char url[1024] = "html_files";
    
    //char url1[] = url_tokens[1];
    //strcat(url,"html_files/");
    concatenate_string(url,url_tokens[1]);
    //printf("URL AFTER STRCAT2 = %s\n",url);
    int exist = stat(url,&buffer1); //check if the file exist
    
    char *source;
    if(exist==-1) //path does not exist
    {
      char path[] = "html_files/error.html";
      char *source;
      read_file(path, &source);
      size_t length = strlen(source);
      char length_text[100];
    //printf("Content-Length: %zu\n",length);
      sprintf(length_text, "%d", length);
      //char content_length[] = length_text;
      char http_header[2048];
      //concatenate_string(http_header,length);
      char version[2048] = "HTTP/1.0 404 Not Found\n";
      char content_type[100]= "Content-Type: text/html\n\n";
      char content_length_name[100]="Content-Length: ";
      //char len[1024] = length;
      concatenate_string(content_length_name,length_text);
      concatenate_string(content_length_name,"\n");
      char extra[100]="\r\n\n";
      concatenate_string(http_header,version);
      concatenate_string(http_header,content_length_name);
      concatenate_string(http_header,content_type);
      concatenate_string(http_header,extra);
      
      //printf("Content-Length: %zu\n",length);
      strcat(http_header,source);
      //printf("HTTP RESPONSE %s\n",http_header);
      write(clientsocket, http_header, sizeof(http_header));
   }
   else
   {
      if(S_ISREG(buffer1.st_mode)==0)
      {
      
      if(url[strlen(url)-1]=='/')
      {
        
        concatenate_string(url,"index.html");
    
      }
      else
      {
      
      concatenate_string(url,"/index.html");

      }
    }
    
    read_file(url, &source);
    size_t length = strlen(source);
    char length_text[100];
    //printf("Content-Length: %zu\n",length);
    sprintf(length_text, "%d", length);
    //char content_length[] = length_text;
    char http_header[2048];
    //concatenate_string(http_header,length);
    char version[2048] = "HTTP/1.0 200 OK\n";
    char content_type[100]= "Content-Type: text/html\n\n";
    char content_length_name[100]="Content-Length: ";
    //char len[1024] = length;
    concatenate_string(content_length_name,length_text);
    concatenate_string(content_length_name,"\n");
    char extra[100]="\r\n\n";
    concatenate_string(http_header,version);
    concatenate_string(http_header,content_length_name);
    concatenate_string(http_header,content_type);
    concatenate_string(http_header,extra);
    strcat(http_header,source);
    //printf("HTTP RESPONSE %s\n",http_header);
    write(clientsocket, http_header, sizeof(http_header));
  }
  

  
  
   
  
  return NULL;
}

// http://localhost:5000/html_files/apart1/flat12/index.html
