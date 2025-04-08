
#include <iostream>    //for cout, endl
#include <unistd.h>    //for fork, pipe
#include <stdlib.h>    //for exit
#include <sys/wait.h>  //for wait
using namespace std;

// This program will fork a child and then execute the "ls -l" command in the child process
// The parent process waits for completion
int main(int argc, char *argv[])
{
   int status;
   int pid, waitPid;

   pid = fork();
   if (pid < 0) 
   {
      cerr << "Error: Fork Failed" << endl;
      exit(EXIT_FAILURE);
   }   
   // This will be the child process
   else if (pid == 0)
   {
      int childPid = getpid();
      cout << "Child " << childPid << " Sleeping " << endl; 
      sleep(5);
      cout << "Child awakes and executes ls" << endl;
      int rc = execlp("/bin/ls", "ls", "-l", (char *) 0);
      if (rc == -1)
      {
         cerr << "Error on execl" << endl;
      }
      cout << "Only see this on error... " << endl;
      exit(EXIT_SUCCESS);
   }
   // This will be the parent
   else
   {
      cout << "Parent Waiting " << endl;
      waitPid = wait(&status);
      if (waitPid == -1)
      {
         cerr << "Error on Wait" << endl;
         exit(EXIT_FAILURE);
      }
      cout << endl << "Parent received Child pid: " << waitPid << endl;
      exit(EXIT_SUCCESS);
   }
}
