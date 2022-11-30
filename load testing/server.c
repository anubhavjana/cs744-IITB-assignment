#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h> // for gettimeofday()

#define thread_pool_size 100

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

struct timeval start, end;
int sockfd;

void *thread_function(void *arg);        // for thread worker pool
void *handle_connection(void *clientfd); // each thread for each client

pthread_t threadpool[thread_pool_size];
int serveropen =1;

struct node
{
  int client_socket;
  struct node *next;
};
typedef struct node node_t;

struct node *head = NULL;
struct node *tail = NULL;
void enqueue(int client_socket)
{
  node_t *newnode = malloc(sizeof(node_t));
  newnode->client_socket = client_socket;
  newnode->next = NULL;

  if (tail == NULL)
  {
    head = newnode;
  }
  else
  {
    tail->next = newnode;
  }
  tail = newnode;
}

int dequeue() // Returns a pointer to a client socket if there is one else returns NULL
{
  if (head == NULL)
    return 0;
  else
  {
    int result = head->client_socket;
    struct node *temp ;
    temp=head;
    head = head->next;
    if (head == NULL)
    {
      tail = NULL;
    }
    free(temp);
    return result;
  }
}

// reads text from path to source
void read_file(char path[], char **source)
{
  FILE *f = fopen(path, "r");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  *source = malloc(fsize + 1);
  fread(*source, fsize, 1, f);
  fclose(f);

  //*source[fsize]='\0'; // this line causes a segfault
}
void concatenate_string(char *s, char *s1)
{
  int i;

  int j = strlen(s);

  for (i = 0; s1[i] != '\0'; i++)
  {
    s[i + j] = s1[i];
  }

  s[i + j] = '\0';

  return;
}

void error(char *msg)
{
  perror(msg);
  // exit(1);
  return;
}


void *thread_function(void *arg)
{

  while (serveropen)
  {
    int pclient;
    pthread_mutex_lock(&mutex);

    if((pclient = dequeue()) == 0 )
    {
      pthread_cond_wait(&condition_var, &mutex); // waiting thread  releases the lock so that other threads can use it
    
      pclient = dequeue();
      

    }
    pthread_mutex_unlock(&mutex);

    if (pclient > 0)
    {
      handle_connection(&pclient);
    }

  }
  pthread_exit(NULL);
  

}

void *handle_connection(void *clientfd)
{
  char url_tokens[50][50];
  int i = 0;
  int clientsocket = *((int *)clientfd);
  int n;
  char buffer[256];

  /* read message from client */
  
  memset(buffer, 0, 256); // zero out the receive buffer to make sure it has null terminated
  n = read(clientsocket, buffer, 255);
  if (n < 0)
    error("CLIENT ERROR reading from socket");
  

  int j = 0;
  int cnt = 0;
  for (i = 0; i <= (strlen(buffer)); i++)
  {
    
    if (buffer[i] == ' ' || buffer[i] == '\0')
    {
      url_tokens[cnt][j] = '\0';
      cnt++; // for next word
      j = 0; // for next word, init index to 0
    }
    else
    {
      url_tokens[cnt][j] = buffer[i];
      j++;
    }
  }

  
  struct stat buffer1;
  char url[1024] = "html_files";

  concatenate_string(url, url_tokens[1]);
  
  int exist = stat(url, &buffer1); // check if the file exist
  char *source=NULL;
  if (exist == -1) // path does not exist
  {
    char path[] = "/error.html";
    char *source=NULL;
    read_file(path, &source);
    size_t length = strlen(source);
    char length_text[100];
    
    sprintf(length_text, "%d", length);
    
    char http_header[2048];
    char version[2048] = "HTTP/1.0 404 Not Found\n";
    char content_type[100] = "Content-Type: text/html\n\n";
    char content_length_name[100] = "Content-Length: ";
    
    concatenate_string(content_length_name, length_text);
    concatenate_string(content_length_name, "\n");
    char extra[100] = "\r\n\n";
    concatenate_string(http_header, version);
    concatenate_string(http_header, content_length_name);
    concatenate_string(http_header, content_type);
    concatenate_string(http_header, extra);

    // printf("Content-Length: %zu\n",length);
    strcat(http_header, source);
    //usleep(1000);
    n = write(clientsocket, http_header, sizeof(http_header));
    if (n < 0)
    {
      error("SERVERERROR writing into  socket");
      close(clientsocket);
    }

    close(clientsocket);
    free(source);
  }
  else
    
  {

    if (S_ISREG(buffer1.st_mode) == 0)
    {

      if (url[strlen(url) - 1] == '/')
      {

        concatenate_string(url, "index.html");
      }
      else
      {

        concatenate_string(url, "/index.html");
      }
    }
      

    read_file(url, &source);
    size_t length = strlen(source);
    char length_text[100];

    sprintf(length_text, "%d", length);

    char http_header[2048];
    bzero(http_header, 2048);
    char version[2048] = "HTTP/1.0 200 OK\n";
    char content_type[100] = "Content-Type: text/html\n\n";
    char content_length_name[100] = "Content-Length: ";

   

    concatenate_string(content_length_name, length_text);
    concatenate_string(content_length_name, "\n");
    char extra[100] = "\r\n\n";
    

    concatenate_string(http_header, version);
        
    concatenate_string(http_header, content_length_name);
    concatenate_string(http_header, content_type);
    concatenate_string(http_header, extra);
    strcat(http_header, source);
    
    
    //usleep(1000);
    n = write(clientsocket, http_header, sizeof(http_header));
    
    if (n < 0)
    {
      error("SERVER ERROR  writing into  socket");
      close(clientsocket);
    }
  }
  close(clientsocket);
  free(source);
  
  return NULL;
}
void sig_handler(int sig)
{
  serveropen=0; //server will close now on CTRL+C
  close(sockfd); //close the listen socket and will stop listening to new requests
  pthread_cond_broadcast(&condition_var); //wake up all threads which were waiting to deque

}

int main(int argc, char *argv[])
{
  
  int i;
  signal(SIGINT,sig_handler);

  // Creating the global arrray of threads in the thread pool
  for (i = 0; i < thread_pool_size; i++)
  {
    pthread_create(&threadpool[i],NULL,&thread_function,NULL);
    
  }

  int  newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;

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
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  

  while (serveropen)
  {
    /* listen for incoming connection requests */

    listen(sockfd, 10000);
    clilen = sizeof(cli_addr);

    
    /* accept a new request, create a newsockfd */
    
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept on CTRL+C");

    int pclient =newsockfd;
    // makes sure only one thread can access the queue at a time
    //  main worker thread creates

    pthread_mutex_lock(&mutex);
    enqueue(pclient);
    pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&mutex);
    
  }

  int j;
  for (j = 0; j < thread_pool_size; j++)
  {
    pthread_join(threadpool[j], NULL);
  }  
    // printf("Head %d\n",head->client_socket);

  //free remaining blocks if any
  if(head!=NULL)
  {
    while(head!=NULL)
    {
      node_t *temp=head;
      head=head->next;
      free(temp);
    }
  }

  return 0;
}