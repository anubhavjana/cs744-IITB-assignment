/* run using: ./load_gen localhost <server port> <number of concurrent users>
   <think time (in s)> <test duration (in s)> */
#include <stdlib.h>
#include<stdio.h>
#include <string.h>
#include <unistd.h>
#include<signal.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <pthread.h>
#include<semaphore.h>
#include <netdb.h>
#include<math.h>
#include <sys/time.h>   // for gettimeofday()

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int time_up;
FILE *log_file;

// user info struct
struct user_info
{
  // user id
  int id;

  // socket info
  int portno;
  char *hostname;
  float think_time;

  // user metrics
  int total_count;
  float total_rtt;
};

// error handling function
void error(char *msg) {
  perror(msg);
  //exit(0);
}

// time diff in seconds
float time_diff(struct timeval *t2, struct timeval *t1) {
  return (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1e6;
}

// user thread function
void *user_function(void *arg)
{
  /* get user info */
  struct user_info *info = (struct user_info *)arg;
      

  int sockfd, n;
  char buffer[2048];
  struct timeval start, end;

  struct sockaddr_in serv_addr;
  struct hostent *server;
  int temp_count=0;
  while (1)
  {
    /* start timer */
    gettimeofday(&start, NULL);
    

    /* TODO: create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    error("ERROR opening socket");
    // printf("opening socket \n");

    /* TODO: set server attrs */
    server = gethostbyname(info->hostname);//gethostbyname(argv[1]);
            //printf("socket open done \n");

    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(info->portno);

    /* TODO: connect to server */

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");



    /* TODO: send message to server */
    bzero(buffer, 2048);
    strcpy(buffer,"GET /index.html HTTP/1.1");
    
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
      error("ERROR writing to socket");
    //memset(buffer,0,2048);
    bzero(buffer,2048);

    /* TODO: read reply from server */
    n = read(sockfd, buffer, 2048);
    if(n==0)
    {
      close(sockfd);
      continue;
    }
    // printf("response arrived\n");
    if (n < 0)
      error("ERROR reading from socket");
    //memset(buffer,0,2048);
    
    
    /* TODO: close socket */
    close(sockfd);
    /* end timer */
    gettimeofday(&end, NULL);
    
    /* if time up, break */
    if (time_up)
      break;

    /* TODO: update user metrics */
   
    info->total_rtt +=  time_diff(&end,&start);
    
    info->total_count++;
    

    /* TODO: sleep for think time */
    
    usleep(info->think_time * 1000000);
  }
  
  //pthread_exit(NULL);
  
  /* exit thread */
  fprintf(log_file, "User #%d finished\n", info->id);
  fflush(log_file);
  //free(info);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int user_count, portno, test_duration;
  float think_time;
  char *hostname;
  

  if (argc != 6) {
    fprintf(stderr,
            "Usage: %s <hostname> <server port> <number of concurrent users> "
            "<think time (in s)> <test duration (in s)>\n",
            argv[0]);
    exit(0);
  }

  hostname = argv[1];
  portno = atoi(argv[2]);
  user_count = atoi(argv[3]);
  think_time = atof(argv[4]);
  test_duration = atoi(argv[5]);

  printf("Hostname: %s\n", hostname);
  printf("Port: %d\n", portno);
  printf("User Count: %d\n", user_count);
  printf("Think Time: %f s\n", think_time);
  printf("Test Duration: %d s\n", test_duration);

  /* open log file */
  log_file = fopen("load_gen.log", "w");

  pthread_t threads[user_count];
  struct user_info info[user_count];
  struct timeval start, end;

  /* start timer */
  gettimeofday(&start, NULL);
  time_up = 0;
  int i;
  for (i = 0; i < user_count; ++i)
  {
    /* TODO: initialize user info */
    info[i].id = i;
    info[i].hostname = hostname;
    info[i].portno = portno;
    info[i].think_time = think_time;
    info[i].total_count = 0;
    info[i].total_rtt=0.0;
    
    
    /* TODO: create user thread */
    // printf("creating %dth thread \n", i);

    pthread_create(&threads[i],NULL,&user_function,&info[i]);
    fprintf(log_file, "Created thread %d\n", i);
  }  
  
  /* TODO: wait for test duration */
  sleep(test_duration);

  fprintf(log_file, "Woke up\n");

  /* end timer */
  time_up = 1;
  gettimeofday(&end, NULL);
  /* TODO:   wait for all threads to finish */
  
  for(int j = 0; j < user_count; j++)
  {
       pthread_join(threads[j], NULL);
  }
  
  //float time_taken =  time_diff(&end,&start);
      
  /* TODO: print results */
  int k=0;
  int req_count=0;
  float rtt_count=0;
  for(k=0;k<user_count;k++)
  {
    req_count+=info[k].total_count;
    rtt_count+=info[k].total_rtt;
    
  }
  float avg_rtt = rtt_count/req_count;
  float throughput = req_count/test_duration;
  printf("Users = %d,Req Count = %d, Total RTT = %f, Avg RTT = %f Throughput = %f\n",user_count,req_count, rtt_count, avg_rtt,throughput);
  
  printf("LoadOutput,%d,%f,%f,%f\n",user_count,rtt_count,avg_rtt,throughput);
  


  /* close log file */
  fclose(log_file);

  return 0;
}
