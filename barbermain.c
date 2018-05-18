// to compile gcc -o barber -std=c99 barbermain.c queue.c -lpthread -lm
#define _REETRANT
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"
// The maximum number of customer threads.
#define MAX_CUSTOMERS_THREADS 50
#define MAX_CUSTOMERS 20
#define COUCH_SEATS 4
#define WAITING_ROOM_SPACE 16

// Function prototypes...
void *customer(void *num);
void *barber(void *);

void randwait(int secs){
    int len;
    // Generate a random number...
    len = (int) ((drand48() * secs) + 1);
    sleep(len);
}

// Define the semaphores.

// waiting room defines the number of persons waiting em pé
sem_t waitingRoom;

// couch Limits the # of customers allowed 
// to sit at the couch at one time.
sem_t couch;   

// barberChair ensures mutually exclusive access to
// the barber chair.
sem_t barberChair;

// barberPillow is used to allow the barber to sleep
// until a customer arrives.
sem_t barberPillow;

// seatBelt is used to make the customer to wait until
// the barber is done cutting his/her hair. 
sem_t seatBelt;

sem_t workbarber;
// Flag to stop the barber thread when all customers
// have been serviced.
int allDone = 0;

sem_t semNextCust;
struct Queue *queueNextCust;

sem_t semNextSofa;
struct Queue *queueNextSofa;

sem_t semNextWr;
struct Queue *queueNextWr;

sem_t semCutHair;
struct Queue *queueCutHair;

sem_t semPaid;
sem_t semPayment;
struct Queue *queuePayment;
sem_t abletopay;
sem_t semGoingHome;
struct Queue *queueGoingHome;

int numcustomersonshop = 0;
sem_t semNumCustomersonshop;

int main(int argc, char *argv[]) {
    pthread_t btid[2];
    pthread_t tid[MAX_CUSTOMERS_THREADS];
    long RandSeed;
    int i, numCustomers;
    int Number[MAX_CUSTOMERS_THREADS], Barbers[3];


    // Check to make sure there are the right number of
    // command line arguments.
    if (argc != 3) {
    printf("Use: SleepBarber <Num Customers> <rand seed>\n");
    exit(-1);
    }

    // Get the command line arguments and convert them
    // into integers.
    numCustomers = atoi(argv[1]);
    RandSeed = atol(argv[2]);

    // Make sure the number of threads is less than the number of
    // customers we can support.
    if (numCustomers > MAX_CUSTOMERS_THREADS) {
    printf("The maximum number of Customers is %d.\n", MAX_CUSTOMERS_THREADS);
    exit(-1);
    }

    printf("\nSleepBarber.c\n\n");
    printf("A solution to the sleeping barber problem using semaphores.\n");

    // Initialize the random number generator with a new seed.
    srand48(RandSeed);

    for (i=0; i<3;i++){
		Barbers[i] = i;
    }

    queueNextWr = createQueue(13);
    queueNextSofa = createQueue(4);
    queueNextCust = createQueue(3);
    queueCutHair = createQueue(3);
    queuePayment = createQueue(3);
    queueGoingHome = createQueue(3);
    // Initialize the semaphores with initial values...
    sem_init(&waitingRoom, 0, WAITING_ROOM_SPACE);
    sem_init(&couch, 0, COUCH_SEATS);
    sem_init(&barberChair, 0, 3);
    sem_init(&barberPillow, 0, 0);
    sem_init(&seatBelt, 0, 0);
    sem_init(&workbarber, 0, 0);

    sem_init(&semNextSofa,0,1);
    sem_init(&semNextCust,0,1);
    sem_init(&semNextWr,0,1);
    sem_init(&semCutHair,0,1);
    sem_init(&semPayment,0,1);
    sem_init(&semPaid,0,1);
    sem_init(&abletopay,0,1);
    sem_init(&semNumCustomersonshop,0,1);
    // Create the barber.
    for(i=0;i<3;i++){
    	pthread_create(&btid[i], NULL, barber, (void *)&Barbers[i]);
    }
        // Initialize the numbers array.
    for (i=0; i<MAX_CUSTOMERS_THREADS; i++){
    	Number[i] = i;
    }
    // Create the customers.
    for (i=0; i<numCustomers; i++) {
       	pthread_create(&tid[i], NULL, customer, (void *)&Number[i]);
    }

    // Join each of the threads to wait for them to finish.
    for (i=0; i<numCustomers; i++) {
    pthread_join(tid[i],NULL);
    }

    // When all of the customers are finished, kill the
    // barber thread.
    allDone = 1;
    sem_post(&barberPillow);  // Wake the barber so he will exit.
    sem_post(&barberPillow);
    sem_post(&barberPillow);
    sleep(1);
    printf("The shop is closed.\n");
    // /for(i=0;i<3;i++){
    // /	pthread_join(btid[i],NULL);    
    // /}
    return 0;
}

void *customer(void *number) {
    int num = *(int *)number;

    // Leave for the shop and take some random amount of
    // time to arrive.
    printf("Customer %d leaving for barber shop.\n", num);
    randwait(5);
    if(numcustomersonshop <= MAX_CUSTOMERS){
        printf("Customer %d arrived at barber shop.\n", num);
        sem_wait(&semNumCustomersonshop);
        numcustomersonshop++;
        sem_post(&semNumCustomersonshop);

        sem_wait(&waitingRoom);
        sem_wait(&semNextWr);
        enqueue(queueNextWr, num);
        printf("Customer %d entering the barbershop.\n", num);
        // Wait for space to open up in the waiting room...
        sem_post(&semNextWr);
        sem_wait(&couch);
        sem_wait(&semNextSofa);
        num = dequeue(queueNextWr);
        sem_post(&waitingRoom);
        enqueue(queueNextSofa, num);
        printf("Customer %d siting at the couch.\n", num);
        // Wait for the barber chair to become free.
        sem_post(&semNextSofa);
        sem_wait(&barberChair);
        // The chair is free so give up your spot in the
        // waiting room.
        sem_wait(&semNextCust);
        num = dequeue(queueNextSofa);
        sem_post(&couch);
        enqueue(queueNextCust, num);
        sem_post(&semNextCust);
        // Wake up the barber...
        printf("Customer %d waking the barber.\n", num);
        sem_post(&barberPillow);
        sem_post(&workbarber);
        // Wait for the barber to finish cutting your hair.
        sem_wait(&seatBelt);

        // Give up the chair.
        sem_post(&barberChair);
        sem_wait(&semCutHair);
        num = dequeue(queueCutHair);
        sem_post(&semCutHair);
        //sem_wait(&semPayment);
        //enqueue(queuePayment, num);
        //sem_post(&semPayment);
        sem_post(&abletopay);
        //sem_wait(&semPaid);
        //sem_wait(&semGoingHome);
        //num = dequeue(queueGoingHome);
        //sem_post(&semGoingHome);
        sem_wait(&semPaid);
        sem_wait(&semNumCustomersonshop);
        numcustomersonshop--;
        sem_post(&semNumCustomersonshop);
        printf("Customer %d leaving barber shop.\n", num);
    }else{
        printf("Shop is full. Customer %d is going home.\n", num);
    }
}

void *barber(void *numberbarber) {
    int numnextcustomer = 0;
    int numbarber = *(int*)numberbarber;
    	// While there are still customers to be serviced...
    	// Our barber is omnicient and can tell if there are 
    	// customers still on the way to his shop.
    	while (!allDone) {
    			
    		// Sleep until someone arrives and wakes you..
    		printf("The barber %d is sleeping \n", numbarber);
    		sem_wait(&barberPillow);
    		
    		// Skip this stuff at the end...
    		if (!allDone) {
    		    // Take a random amount of time to cut the
    		    // customer's hair.
    		    sem_wait(&semNextCust);
    		    numnextcustomer = dequeue(queueNextCust);
    		    sem_post(&semNextCust);
    		    printf("The barber %d is cutting hair of %d.\n", numbarber, numnextcustomer);
    		    sem_wait(&workbarber);
    		    randwait(3);
    		    printf("The barber %d has finished cutting hair.\n", numbarber);
    		    sem_wait(&semCutHair);
    		    enqueue(queueCutHair, numnextcustomer);
    		    sem_post(&semCutHair);
    		    // Release the customer when done cutting...
    		    sem_post(&seatBelt);
    		    //numnextcustomer = 0;
    		    sem_wait(&abletopay);
    		    //sem_wait(&semPayment);
    		    //numnextcustomer = dequeue(queuePayment);
    		    //sem_post(&semPayment);
    		    printf("The barber %d is receiving from the customer %d.\n", numbarber, numnextcustomer);
    		    randwait(3);
    		    //sem_wait(&semGoingHome);
    		    //enqueue(queueGoingHome, numnextcustomer);
    		    //sem_post(&semGoingHome);
    		    sem_post(&semPaid);
    		    sem_post(&abletopay);
    		    numnextcustomer = 0;
    	}else {
    	    printf("The barber %d is going home for the day.\n", numbarber);
    	}
    }
}