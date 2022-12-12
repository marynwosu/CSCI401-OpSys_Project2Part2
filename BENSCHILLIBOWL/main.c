#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "BENSCHILLIBOWL.h"

// Feel free to play with these numbers! This is a great way to
// test your implementation.
#define BENSCHILLIBOWL_SIZE 10
#define NUM_CUSTOMERS 3
#define NUM_COOKS 2
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS NUM_CUSTOMERS * ORDERS_PER_CUSTOMER

// Global variable for the restaurant.
BENSCHILLIBOWL *bcb;

/**
 * Thread funtion that represents a customer. A customer should:
 *  - allocate space (memory) for an order.
 *  - select a menu item.
 *  - populate the order with their menu item and their customer ID.
 *  - add their order to the restaurant.
 */
void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = (int)(long) tid;
		int i;

		Order* o;

		for (i=0; i<ORDERS_PER_CUSTOMER; i++) {
			o = (Order*) malloc(sizeof(Order));
			o->menu_item = PickRandomMenuItem();
			o->customer_id = customer_id;
			o->order_number = AddOrder(bcb, o);
      printf("Customer #%d added Order #%d\n", customer_id, o->order_number);

      o->next = NULL;
		}
}

/**
 * Thread function that represents a cook in the restaurant. A cook should:
 *  - get an order from the restaurant.
 *  - if the order is valid, it should fulfill the order, and then
 *    free the space taken by the order.
 * The cook should take orders from the restaurants until it does not
 * receive an order.
 */
void* BENSCHILLIBOWLCook(void* tid) {
  int cook = (int)(long) tid;
	int orders_fulfilled = 0;
	
	Order* o = GetOrder(bcb);
	while (o != NULL) {
		free (o);
		orders_fulfilled++;
 		o = GetOrder(bcb);
	}
 
  printf("Cook #%d fulfilled %d orders\n", cook, orders_fulfilled);
}

/**
 * Runs when the program begins executing. This program should:
 *  - open the restaurant
 *  - create customers and cooks
 *  - wait for all customers and cooks to be done
 *  - close the restaurant.
 */
int main() {

  //  NUM_CUSTOMERS 90
  // 	NUM_COOKS 10
  // #define ORDERS_PER_CUSTOMER 3
  // #define  NUM_CUSTOMERS * ORDERS_PER_CUSTOMER
	int i;
	pthread_t customer[NUM_CUSTOMERS];
	pthread_t cook[NUM_COOKS];

	int cust_id[NUM_CUSTOMERS], cook_id[NUM_COOKS];

	bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);

	// create customers thread
	for (i=0; i < NUM_CUSTOMERS; i++) {
		cust_id[i] = i;
    // Create a thread that will function BENSCHILLIBOWLCustomer()
    int err = pthread_create(&customer[i], NULL, &BENSCHILLIBOWLCustomer, &(cust_id[i]));

    // Check if thread is created sucessfuly
    if (err != 0) {
        printf("Thread creation failed\n");
        exit(1);
    }
 
	}

	// create cooks thread
	for (i=0; i < NUM_COOKS; i++) {
		cook_id[i] = i;
		// Create a thread that will function BENSCHILLIBOWLCook()
    int err = pthread_create(&cook[i], NULL, &BENSCHILLIBOWLCook, &(cook_id[i]));

    // Check if thread is created sucessfuly
    if (err != 0) {
        printf("Thread creation failed\n");
        exit(1);
    }	
	}

	// join method for customers
	for (i=0; i < NUM_CUSTOMERS; i++) {
		pthread_join(customer[i], NULL);
	}

	// join method for cooks
	for (i=0; i < NUM_COOKS; i++) {
		pthread_join(cook[i], NULL);
	}

	CloseRestaurant(bcb);
  return 0;
}