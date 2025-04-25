// --------------------------------------------shop.cpp--------------------------------------------
// Aldan Brown CSS 503
// Date Created: 4/25/2025
// Date Modified: 4/25/2025
// ------------------------------------------------------------------------------------------------
// Description: Implementation of shop.h
// ------------------------------------------------------------------------------------------------

#include "shop.h"

// ------------------------------------Constructors/Destructor-------------------------------------
// Constructor
Shop::Shop()
    : max_barbers_(kDefaultBarbers), max_waiting_cust_(kDefaultNumChairs), customer_in_chair_(0),
      in_service_(false), money_paid_(false), cust_drops_(0) {
   init();
}

// Constructor (int, int)
Shop::Shop(const int num_barbers, const int num_chairs)
    : max_barbers_((num_barbers > 0) ? num_barbers : kDefaultBarbers),
      max_waiting_cust_((num_chairs > 0) ? num_chairs : kDefaultNumChairs), customer_in_chair_(0),
      in_service_(false), money_paid_(false), cust_drops_(0) {
   init();
}

void Shop::init() {
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   pthread_cond_init(&cond_customer_served_, NULL);
   pthread_cond_init(&cond_barber_paid_, NULL);
   pthread_cond_init(&cond_barber_sleeping_, NULL);
}

string Shop::int2string(int i) {
   stringstream out;
   out << i;
   return out.str();
}

void Shop::print(int person, string message) {
   cout << ((person != barber) ? "customer[" : "barber  [") << person << "]: " << message << endl;
}

int Shop::get_cust_drops() const { return cust_drops_; }

bool Shop::visitShop(int id) {
   pthread_mutex_lock(&mutex_);

   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_) {
      print(id, "leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return false;
   }

   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service
   if (customer_in_chair_ != 0 || !waiting_chairs_.empty()) {
      waiting_chairs_.push(id);
      print(id, "takes a waiting chair. # waiting seats available = " +
                    int2string(max_waiting_cust_ - waiting_chairs_.size()));
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
      waiting_chairs_.pop();
   }

   print(id, "moves to the service chair. # waiting seats available = " +
                 int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_ = id;
   in_service_ = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_);

   pthread_mutex_unlock(&mutex_);
   return true;
}

void Shop::leaveShop(int id) {
   pthread_mutex_lock(&mutex_);

   // Wait for service to be completed
   print(id, "wait for the hair-cut to be done");
   while (in_service_ == true) {
      pthread_cond_wait(&cond_customer_served_, &mutex_);
   }

   // Pay the barber and signal barber appropriately
   money_paid_ = true;
   pthread_cond_signal(&cond_barber_paid_);
   print(id, "says good-bye to the barber.");
   pthread_mutex_unlock(&mutex_);
}

void Shop::helloCustomer() {
   pthread_mutex_lock(&mutex_);

   // If no customers than barber can sleep
   if (waiting_chairs_.empty() && customer_in_chair_ == 0) {
      print(barber, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   if (customer_in_chair_ == 0) // check if the customer, sit down.
   {
      pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   print(barber, "starts a hair-cut service for " + int2string(customer_in_chair_));
   pthread_mutex_unlock(&mutex_);
}

void Shop::byeCustomer() {
   pthread_mutex_lock(&mutex_);

   // Hair Cut-Service is done so signal customer and wait for payment
   in_service_ = false;
   print(barber, "says he's done with a hair-cut service for " + int2string(customer_in_chair_));
   money_paid_ = false;
   pthread_cond_signal(&cond_customer_served_);
   while (money_paid_ == false) {
      pthread_cond_wait(&cond_barber_paid_, &mutex_);
   }

   // Signal to customer to get next one
   customer_in_chair_ = 0;
   print(barber, "calls in another customer");
   pthread_cond_signal(&cond_customers_waiting_);

   pthread_mutex_unlock(&mutex_); // unlock
}
