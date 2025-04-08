
/*
* example program which shows usage of a pipe for communication
*/

#include <stdlib.h>     //exit
#include <stdio.h>      //perror
#include <unistd.h>     //fork, pipe
#include <sys/wait.h>   //wait
#include <iostream>
using namespace std;

const int BUF_SIZE = 4096;

int main()
{
   enum {READ, WRITE};
   pid_t pid;
   int pipeFD[2];

   if (pipe(pipeFD) < 0)
   {
      perror("Error in creating pipe");
      exit(EXIT_FAILURE);
   }

   pid = fork();
   if (pid < 0)
   {
      perror("Error during fork");
      exit(EXIT_FAILURE);
   }

   if (pid == 0)  //Child
   {
      // Closing the read side of the pipe and writing out stdout into the pipe
      // Everything which normally would go to the console (stdout) now goes into the pipe
      close(pipeFD[READ]); 
      dup2(pipeFD[WRITE], 1);   //stdout is now child's read pipe
      execlp("/bin/ls", "ls", "-l", NULL);
      // process is overlayed so does not execut past here...
   }
   else   //Parent 
   {
      int status;
      int childPid = wait(&status);

      // Here the parent will read what the child has written into the pipe
      // In the case of a Shell the stdin would be overwritten on the read side so that 
      // output from the child process would go into input of the parent.
      char buf[BUF_SIZE];
      int n = read(pipeFD[READ], buf, BUF_SIZE);
      buf[n] = '\0';
      cout << buf;
      cout << "Parent exiting" <<endl;
   }
   exit(EXIT_SUCCESS);
} 
