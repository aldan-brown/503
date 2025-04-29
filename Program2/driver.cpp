// --------------------------------------------driver.cpp--------------------------------------------
// Aldan Brown CSS 503
// Date Created: 4/25/2025
// Date Modified: 4/29/2025
// ------------------------------------------------------------------------------------------------
// Description: Main file running shop.h/shop.cpp barber shop programs. Contains threads for
// barbers and maintains lists of customers and chairs.
// ------------------------------------------------------------------------------------------------
// Acknowledgements: Initial code provided by Prof. Robert Dimpsey on 4/25/25
// ------------------------------------------------------------------------------------------------

#include "shop.h"
#include <iostream>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

void* barber(void*);
void* customer(void*);

// ThreadParam class
// This class is used as a way to pass more
// than one argument to a thread.
class ThreadParam {
 public:
   ThreadParam(Shop* shop, int id, int service_time)
       : shop(shop), id(id), service_time(service_time) {};
   Shop* shop;
   int id;
   int service_time;
};

// main(int, char*[])
int main(int argc, char* argv[]) {

   // Read arguments from command line
   // TODO: Validate values
   if (argc != 5) {
      cout << "Usage: num_barbers num_chairs num_customers service_time" << endl;
      return -1;
   }

   int num_barbers = atoi(argv[1]);
   int num_chairs = atoi(argv[2]);
   int num_customers = atoi(argv[3]);
   int service_time = atoi(argv[4]);

   // Many barbers, one shop, many customers
   pthread_t barber_thread[num_barbers];
   pthread_t customer_threads[num_customers];
   Shop shop(num_barbers, num_chairs);

   // Create barbers threads
   for (int i = 0; i < num_barbers; i++) {
      ThreadParam* barber_param = new ThreadParam(&shop, i, service_time);
      pthread_create(&barber_thread[i], NULL, barber, barber_param);
   }

   // Create customer threads
   for (int i = 0; i < num_customers; i++) {
      usleep(rand() % 1000);
      int id = i + 1;
      ThreadParam* customer_param = new ThreadParam(&shop, id, 0);
      pthread_create(&customer_threads[i], NULL, customer, customer_param);
   }

   // Wait for customers to finish and cancel barber
   for (int i = 0; i < num_customers; i++) {
      pthread_join(customer_threads[i], NULL);
   }

   for (int i = 0; i < num_barbers; i++) {
      pthread_cancel(barber_thread[i]);
   }

   cout << "# customers who didn't receive a service = " << shop.get_cust_drops() << endl;
   return 0;
}

// barber(void*)
void* barber(void* arg) {
   // extract parameters
   ThreadParam& param = *(ThreadParam*)arg;
   Shop& shop = *(param.shop);
   int id = param.id;
   int service_time = param.service_time;
   delete &param;
   // keep working until being terminated by the main
   while (true) {
      shop.helloCustomer(id); // pick up a new customer
      usleep(service_time);
      shop.byeCustomer(id); // release the customer
   }

   return NULL;
}

// customer(void*)
void* customer(void* arg) {
   ThreadParam& param = *(ThreadParam*)arg;
   Shop& shop = *(param.shop);
   int id = param.id;
   delete &param;
   // if assigned to barber i then wait for service to finish
   // -1 means did not get barber
   int barber = -1;
   if ((barber = shop.visitShop(id)) != -1) {
      shop.leaveShop(id, barber); // wait until my service is finished
   }

   return NULL;
}
