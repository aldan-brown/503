// ---------------------------------------------shop.h---------------------------------------------
// Aldan Brown CSS 503
// Date Created: 4/25/2025
// Date Modified: 4/25/2025
// ------------------------------------------------------------------------------------------------
// Description: This program implements an extention of the barbershop problem.
// A barbershop consists of a waiting room with n waiting chairs and a barber room with m barber
// chairs. If there are no customers to be served, all the barbers go to sleep. If a customer
// enters the barbershop and all chairs (including both, waiting and barber chairs) are occupied,
// then the customer leaves the shop. If all the barbers are busy but chairs are available, then
// the customer sits in one of the free waiting chairs. If the barbers are asleep,the customer
// wakes up one of the barbers.
// ------------------------------------------------------------------------------------------------
// Assumptions:
//
// ------------------------------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <pthread.h>
#include <queue>
#include <sstream>
#include <string>

using namespace std;

/** Establish default constants for the number of barbers and waiting room chairs*/
#define kDefaultNumChairs 3
#define kDefaultBarbers 1

class Shop {
 public:
   // -----------------------------------Constructors/Destructor-----------------------------------
   /** Default constructor */
   Shop();

   /** Alternate constructor
   @param num_barbers the total number of barbers that can serve customers
   @param num_chairs the total number of chairs in the waiting room */
   Shop(const int num_barbers, const int num_chairs);

   /** Destructor */
   ~Shop();

   // --------------------------------------Public Functions---------------------------------------

   /** Executes shop visit by a customer
   @param id the customer's id number
   @return returns true when a customer gets a service, false otherwise */
   bool visitShop(int id);

   /** Finishes a customer visit
   @param customer_id the customer to be served
   @param barber_id either the id of the barber who served the cusomter or -1 if none available */
   void leaveShop(int customer_id, int barber_id);



   void helloCustomer(int id);

   void byeCustomer(int id);

   int get_cust_drops() const;

   // ---------------------------------------------------------------------------------------------

 private:
   const int max_waiting_cust_; // the max number of threads that can wait
   const int max_barbers_;      // the max number of barbers that can serve
   int customer_in_chair_;
   bool in_service_;
   bool money_paid_;
   queue<int> waiting_chairs_; // includes the ids of all waiting threads
   int cust_drops_;

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;
   pthread_cond_t cond_customers_waiting_;
   pthread_cond_t cond_customer_served_;
   pthread_cond_t cond_barber_paid_;
   pthread_cond_t cond_barber_sleeping_;

   static const int barber = 0; // the id of the barber thread

   void init();
   string int2string(int i);
   void print(int person, string message);
};
