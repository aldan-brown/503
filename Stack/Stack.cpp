#include "Stack.h"

Stack::Stack() { 
   head = nullptr;
   pthread_mutex_init(&lock, NULL);
 }

bool Stack::push(int val) {
   Node* insNode = new Node();
   insNode->value = val;
   insNode->next = nullptr;
   // mutex lock - first access of stack is to assign new node to the front
   pthread_mutex_lock(&lock);
   insNode->next = head;
   head = insNode;
   // mutex unlock
   pthread_mutex_unlock(&lock);
   return true;
}

bool Stack::pop(int& val) {
   // mutex lock - first access of stack is to check head value
   pthread_mutex_lock(&lock);
   if (head != nullptr) {
      val = head->value;
      Node* temp = head;
      head = head->next;
      // mutex unlock - if true
      pthread_mutex_unlock(&lock);
      delete temp;
      return true;
   } else {
      // mutex unlock - if false, release immediately
      pthread_mutex_unlock(&lock);
      return false;
   }
}

Stack::~Stack() {
   int val;
   while (pop(val))
      ;
}
