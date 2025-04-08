// --------------------------------------------lab1.cpp--------------------------------------------
// Aldan Brown CSS 503
// Date: 4/8/2025
// ------------------------------------------------------------------------------------------------

#include <iostream>    // cout, cerr
#include <queue>       // stl queue
#include <sys/types.h> // fork, wait
#include <sys/wait.h>  // wait
#include <unistd.h>    // fork

#include <stdlib.h> // for exit
using namespace std;

queue<int> pids; // stores a list of processes from the parent to a leaf process

void recursive_creation(int leaves) {
   if (leaves > 1) {
      pids.push(getpid());

      // fork a left child
      int leftChild = fork();

      if (leftChild == 0) {
         recursive_creation(leaves - leaves / 2);
         exit(0);
      }

      int status;
      // wait for one of the children
      wait(&status);

      // fork a right child
      int rightChild = fork();
      if (rightChild == 0) {
         recursive_creation(leaves / 2);
         exit(0);
      }

      // wait for the other child
      wait(&status);

      exit(0);

   } else {                     // I'm a leaf process
      while (pids.size() > 0) { // print out a list of my ascendants
         cout << pids.front() << " ";
         pids.pop();
      }
      cout << getpid() << endl; // print out myself
      exit(0);
   }
}

int main(int argc, char* argv[]) {
   // validate arguments
   if (argc != 2) {
      cerr << "usage: lab1 #leaves" << endl;
      return -1;
   }

   int leaves = atoi(argv[1]);
   if (leaves < 1) {
      cerr << "usage: lab1 #leaves" << endl;
      cerr << "where #leaves >= 1" << endl;
      return -1;
   }

   recursive_creation(leaves);
   return 0;
}
