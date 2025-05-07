//--------------------------------------------lab2.cpp---------------------------------------------
// CSS 503 - Lab 2
// Created by: Aldan Brown and Matthew Van Ginneken
// Date:4/28/2025
//-------------------------------------------------------------------------------------------------

#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

int nThreads; // #threads

int turn;              // turn points which thread should run
pthread_mutex_t mutex; // a lock for this critical section
pthread_cond_t* cond;  // array of condition variable[nThreads]

void* thread_func(void* arg) {
   int id = ((int*)arg)[0]; // this thread's identifier
   delete (int*)arg;

   for (int loop = 0; loop < 10; loop++) // Run 10 times
   {
      // enter the critical section
      pthread_mutex_lock(&mutex);

      while (turn != id) {
         pthread_cond_wait(&cond[id], &mutex); // wait until the (id - 1)th thread signals me.
      }

      cout << "thread[" << id << "] got " << loop << "th turn" << endl;

      // signal the next thread
      turn = (id + 1) % nThreads;       // Update the turn using modulation to loop
      pthread_cond_signal(&cond[turn]); // Signal next thread

      // leave the critical section
      pthread_mutex_unlock(&mutex);
   }
   return NULL;
}

int main(int argc, char* argv[]) {
   if (argc != 2) {
      cerr << "usage: lab2 #threads" << endl;
      return -1;
   }

   nThreads = atoi(argv[1]);
   if (nThreads < 1) {
      cerr << "usage: lab1 #threads" << endl;
      cerr << "where #threads >= 1" << endl;
      return -1;
   }

   pthread_mutex_init(&mutex, NULL);
   pthread_t* tid = new pthread_t[nThreads]; // an array of thread identifiers
   cond = new pthread_cond_t[nThreads];      // an array of condition variables
   turn = 0;                                 // points to which thread should run

   for (int i = 0; i < nThreads; i++) {
      int* id = new int[1];
      id[0] = i;
      pthread_create(&tid[i], NULL, thread_func, (void*)id);
   }

   for (int i = 0; i < nThreads; i++) // wait for all threads
   {
      pthread_join(tid[i], NULL);
   }
}