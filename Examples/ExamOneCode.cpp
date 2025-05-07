#include <iostream>   // for cout, endl
#include <pthread.h>  // for pthreads
#include <queue>      // for queues
#include <sstream>    // for string streams
#include <stdlib.h>   // for exit
#include <string>     // for string
#include <sys/wait.h> // for wait
#include <unistd.h>   // for fork, pipe
#include <vector>     // for vector
using namespace std;

pthread_mutex_t mutex; // a lock for this critical section
pthread_cond_t* cond;  // array of condition variable[nThreads]
queue<int> idOrder;
int numThreads;

void* examThread(void* arg) {
   int id = *static_cast<int*>(arg); // Convert void* back to int*
   delete static_cast<int*>(arg);

   pthread_mutex_lock(&mutex);

   while (idOrder.size() < (id + 1)) {
      pthread_cond_wait(&cond[id], &mutex);
   }

   pid_t thisPid = fork();

   if (thisPid == 0) {
      idOrder.push(id);
      cout << "Child " << id << " added" << endl;
   } else {
      wait(NULL);
      idOrder.push(id);
      cout << "Parent " << id << " added" << endl;
      pthread_mutex_unlock(&mutex);

      if (id + 1 < numThreads) {
         pthread_cond_signal(&cond[id + 1]);
      }
   }

   if(thisPid == 0){
      exit(0);
   }

   return nullptr;
}

int main(int argc, char* argv[]) {
   if (argc != 2) {
      cerr << "usage: ExamOneCode #threads" << endl;
      return -1;
   }

   numThreads = atoi(argv[1]);

   if (numThreads < 1) {
      cerr << "usage: ExamOneCode #threads" << endl;
      cerr << "where  #threads >= 1" << endl;
      return -1;
   }

   pthread_mutex_init(&mutex, NULL);
   pthread_t* tid = new pthread_t[numThreads]; // an array of thread identifiers
   cond = new pthread_cond_t[numThreads];

   for (int i = 0; i < numThreads; i++) {
      pthread_cond_init(&cond[i], nullptr);
   }

   // Prime the first condition so thread 0 can run
   idOrder.push(-1); // simulate that the condition for thread 0 is satisfied

   for (int i = 0; i < numThreads; i++) {
      int* id = new int[1];
      id[0] = i;
      pthread_create(&tid[i], NULL, examThread, (void*)id);
   }

   for (int i = 0; i < numThreads; i++) // wait for all threads
   {
      pthread_join(tid[i], NULL);
   }

   delete[] tid;
   delete[] cond;

   return 0;
}
