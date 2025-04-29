// --------------------------------------------shop.cpp--------------------------------------------
// Aldan Brown CSS 503
// Date Created: 4/25/2025
// Date Modified: 4/29/2025
// ------------------------------------------------------------------------------------------------
// Description: Implementation of shop.h
// ------------------------------------------------------------------------------------------------

#include "shop.h"

// ------------------------------------Constructors/Destructor-------------------------------------
// Constructor
Shop::Shop() : max_barbers_(kDefaultBarbers), max_waiting_cust_(kDefaultNumChairs), cust_drops_(0) {
   customer_in_chair_.resize(max_barbers_, 0);
   // Resize vectors
   in_service_.resize(max_barbers_, false);
   money_paid_.resize(max_barbers_, false);
   // Initialize threads
   init();
}

// Constructor (int, int)
Shop::Shop(const int num_barbers, const int num_chairs)
    : max_barbers_((num_barbers > 0) ? num_barbers : kDefaultBarbers),
      max_waiting_cust_((num_chairs > 0) ? num_chairs : kDefaultNumChairs), cust_drops_(0) {
   // Resize vectors
   customer_in_chair_.resize(max_barbers_, 0);
   in_service_.resize(max_barbers_, false);
   money_paid_.resize(max_barbers_, false);
   // Initialize threads
   init();
}

// Destructor
Shop::~Shop() {
   pthread_mutex_destroy(&mutex_);
   pthread_cond_destroy(&cond_customers_waiting_);
   for (int i = 0; i < max_barbers_; ++i) {
      pthread_cond_destroy(&cond_customer_served_[i]);
      pthread_cond_destroy(&cond_barber_paid_[i]);
      pthread_cond_destroy(&cond_barber_sleeping_[i]);
   }
}

// --------------------------------------Public Functions---------------------------------------

// visitShop(int)
int Shop::visitShop(const int id) {
   // Mutex lock
   pthread_mutex_lock(&mutex_);

   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_) {
      print(id, "leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return -1;
   }

   // Find next available barber id
   int assigned_barber = -1;
   for (int i = 0; i < max_barbers_; ++i) {
      if (customer_in_chair_[i] == 0) {
         assigned_barber = i;
         break; // Exit early if empty barber chair found
      }
   }

   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service
   if (assigned_barber == -1) {
      // Customer takes waiting chair
      waiting_chairs_.push(id);
      print(id, "takes a waiting chair. # waiting seats available = " +
                    int2string(max_waiting_cust_ - waiting_chairs_.size()));
      // Wait for next barber
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
      // Open chair available signal received
      waiting_chairs_.pop();

      // Check chairs again for first available barber
      for (int i = 0; i < max_barbers_; ++i) {
         if (customer_in_chair_[i] == 0) {
            assigned_barber = i;
            break;
         }
      }
   }
   // Add new customer to barber chair
   print(id, "moves to the service chair. # waiting seats available = " +
                 int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_[assigned_barber] = id;
   in_service_[assigned_barber] = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_[assigned_barber]);

   // Mutex unlock
   pthread_mutex_unlock(&mutex_);
   return assigned_barber;
}

// leaveShop(int)
void Shop::leaveShop(const int customer_id, const int barber_id) {
   pthread_mutex_lock(&mutex_);

   // Wait for service to be completed
   print(customer_id, "wait for the hair-cut to be done");
   while (in_service_[barber_id] == true) {
      pthread_cond_wait(&cond_customer_served_[barber_id], &mutex_);
   }

   // Pay the barber and signal barber appropriately
   money_paid_[barber_id] = true;
   pthread_cond_signal(&cond_barber_paid_[barber_id]);
   print(customer_id, "says good-bye to the barber.");
   pthread_mutex_unlock(&mutex_);
}

// helloCustomer(int)
void Shop::helloCustomer(const int id) {
   // Mutex lock
   pthread_mutex_lock(&mutex_);

   // If no customers in the shop than barber can sleep
   if (waiting_chairs_.empty() && customer_in_chair_[id] == 0) {
      print(-id, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }

   // Waits for customer to sit down
   if (customer_in_chair_[id] == 0) // check if the customer, sit down.
   {
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }
   // Customer sits and begins service
   print(-id, "starts a hair-cut service for " + int2string(customer_in_chair_[id]));
   // Mutex unlock
   pthread_mutex_unlock(&mutex_);
}

// byeCustomer(int)
void Shop::byeCustomer(const int id) {
   // Mutex lock
   pthread_mutex_lock(&mutex_);

   // Hair Cut-Service is done so signal customer and wait for payment
   in_service_[id] = false;
   print(-id,
         "says he's done with a hair-cut service for " + int2string(customer_in_chair_[id]));
   money_paid_[id] = false;
   pthread_cond_signal(&cond_customer_served_[id]);
   while (money_paid_[id] == false) {
      pthread_cond_wait(&cond_barber_paid_[id], &mutex_);
   }

   // Signal to customer to get next one
   customer_in_chair_[id] = 0;
   print(-id, "calls in another customer");
   pthread_cond_signal(&cond_customers_waiting_);

   pthread_mutex_unlock(&mutex_); // unlock
}

// get_cust_drops()
int Shop::get_cust_drops() const { return cust_drops_; }

//---------------------------------------Private Functions--------------------------------------

// init()
void Shop::init() {
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   cond_customer_served_.resize(max_barbers_);
   cond_barber_paid_.resize(max_barbers_);
   cond_barber_sleeping_.resize(max_barbers_);

   for (int i = 0; i < max_barbers_; i++) {
      pthread_cond_init(&cond_customer_served_[i], NULL);
      pthread_cond_init(&cond_barber_paid_[i], NULL);
      pthread_cond_init(&cond_barber_sleeping_[i], NULL);
   }
}

// int2string(int)
string Shop::int2string(const int i) {
   stringstream out;
   out << i;
   return out.str();
}

// print(int, string)
void Shop::print(int person, string message) const {
   cout << ((person != barber) ? "customer[" : "barber  [") << person << "]: " << message << endl;
}
