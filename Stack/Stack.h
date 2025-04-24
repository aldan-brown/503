// --------------------------------------------Stack.h---------------------------------------------
// Aldan Brown CSS 503
// Date: 4/24/2025
// ------------------------------------------------------------------------------------------------

#pragma once
#include <pthread.h>

typedef struct Node {
   int value;
   Node* next;
};

class Stack {
 public:
   Stack();
   bool push(int val);
   bool pop(int& val);
   ~Stack();

 private:
   Node* head;
   pthread_mutex_t lock;
   
};
