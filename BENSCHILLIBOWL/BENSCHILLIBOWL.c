#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    int r_ind = rand() % BENSCHILLIBOWLMenuLength;
    return BENSCHILLIBOWLMenu[r_ind];
}

/* Allocate memory for the Restaurant, then create the mutex and 
condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL* bcb;
    bcb = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
    bcb->orders = NULL;
    bcb->current_size = 0;
    bcb->max_size = max_size;
    bcb->next_order_number = 1;
    bcb->orders_handled = 0;
	  bcb->expected_num_orders = expected_num_orders;

    pthread_mutex_init(&(bcb->mutex), NULL);
    pthread_cond_init(&(bcb->can_add_orders), NULL);
    pthread_cond_init(&(bcb->can_get_orders), NULL);

    srand(getpid());
    printf("Restaurant is open!\n");
    return bcb;
}


/* check that the number of orders received is equal to the number handled 
(ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    printf("Orders handled: %d\n", bcb->orders_handled);
    printf("Orders expected: %d\n", bcb->expected_num_orders);
    if(bcb->expected_num_orders == bcb->orders_handled){
       pthread_mutex_destroy(&bcb->mutex);
       pthread_cond_destroy(&bcb->can_add_orders);
       pthread_cond_destroy(&bcb->can_get_orders);
    }
    free(bcb);
    printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    // lock.acquire();
    // while (queue.isFull())
    // empty.wait(&lock);
    // queue.enqueue(item);
    // full.signal();
    // lock.release();

    pthread_mutex_lock(&(bcb->mutex)); 
    while (IsFull(bcb)) { 
        pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    }
    order->order_number = bcb->next_order_number;
    AddOrderToBack(&(bcb->orders), order);
    bcb->next_order_number += 1; 
    bcb->current_size += 1;
    pthread_cond_broadcast(&(bcb->can_get_orders));
    pthread_mutex_unlock(&(bcb->mutex));

    return bcb->next_order_number - 1;
}

/* get an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {

    // lock.acquire();
    // while (queue.isEmpty())
    // full.wait(&lock);
    // item = queue.dequeue(item);
    // empty.signal();
    // lock.release();
    // return item;

    pthread_mutex_lock(&(bcb->mutex));
    while(IsEmpty(bcb)) { 
        if (bcb->orders_handled >= bcb->expected_num_orders) {
            pthread_mutex_unlock(&(bcb->mutex));
            return NULL;
        }
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    }
    
    // get order from queue(FIFO).
    Order *order = bcb->orders;
    bcb->orders = bcb->orders->next;
    
    // update the current order size and orders handled
    bcb->current_size -= 1; 
    bcb->orders_handled += 1;
    
    // notify the process that it add get orders now
    pthread_cond_broadcast(&(bcb->can_add_orders));
        
    // release the lock.
    pthread_mutex_unlock(&(bcb->mutex)); 

    return order;
}

// Optional helper functions (you can implement if you think they would be useful)
bool IsEmpty(BENSCHILLIBOWL* bcb) {
    if (bcb->current_size == 0)
        return true;
    return false;
}

bool IsFull(BENSCHILLIBOWL* bcb) {
    if (bcb->max_size == bcb->current_size)
        return true;
    return false;
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders == NULL) {
      *orders = order;
  } 
  else {
      Order *curr_order = *orders;
      while (curr_order->next) {
          curr_order = curr_order->next;
      }
      curr_order->next = order;
    }
}

