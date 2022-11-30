#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i,status;
  int bg_process_id;


	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
    int k;
    int length=0;
    for ( k = 0; tokens[k]!=NULL;k++)
    {
      length++;
    }
    
    
    //printf("Length of the token = %d\n",length);
    
		if(tokens[0] == NULL)
		{
			continue;
		}

		 int fc=fork();
		 if(fc<0)
		{
			fprintf(stderr,"%s\n","Unable to create child process !!\n");

		}
		else if(fc==0) //Child process
		{
      
      printf("COMMAND = %s\n",tokens[0]);
      if(strcmp(tokens[0],"exit") == 0)
      {
        int x = kill(getppid(), SIGKILL);
        
      }
      /* IMPLEMENTING "cd" via CHDIR() -> Prints the path of the directory , shows current working directory 
        and after chdir() displays new working directory */

		  if(strcmp(tokens[0],"cd") == 0)
		  {
        
        printf("PATH for cd = %s\n",tokens[1]);
        char s[100];
    		printf("Now current is :  %s\n",getcwd(s,100));
    	
    		int chdir_exit_status = chdir(tokens[1]);
        
        if(chdir_exit_status==0)
        {
          printf("Directory succesfully changed to = %s\n",getcwd(s,100));
        }
        if(chdir_exit_status<0)
        {
          printf("Cd command failed !!");
        }
      }
      if(strcmp(tokens[length-1],"&") == 0)
      {
        tokens[length-1]='\0';
        execvp(tokens[0],tokens);
        
      }
      
       
		 else
      // If the command is not "cd" 
		  {
			  execvp(tokens[0],tokens);
			  //printf("Command execution failed\n");
			  _exit(1);
		  }
		}
    
		else
		{
      
      if(strcmp(tokens[length-1],"&") == 0) // User has entered & to run it in background
      {
        printf("The process id of the child to move to background is : %d\n",fc);
        int bg_process_id =fc;

        int wait_pid_result_1 = waitpid(bg_process_id,&status,WEXITED);
        int wait_pid_result_2 = waitpid(bg_process_id,&status,WNOHANG);
        

        while(wait_pid_result_1==0)
        {
          sleep(10);
          if()
          printf("Background process %d completed\n",bg_process_id);
        }
        
       

        printf("The result of waitpid is %d and the status is: %d\n",wait_pid_result_1,status);
      
        
      }
      // If its an "exit" command then , parent will not WAIT(), it will kill() 
      if(strcmp(tokens[0],"exit") == 0)
      {
        int x = kill(getpid(), SIGKILL);

        
      }
      else
      {
			int wc = wait(NULL); //parent process wait till child process completes.
      }
		}
       
         // Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
