/* 
 * stoplight.c
 *
 * You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>
#include <queue.h>

//locks:
struct lock * NW = NULL;
struct lock * NE = NULL;
struct lock * SW = NULL;
struct lock * SE = NULL;
int fromNorthOccupied = -1, fromEastOccupied = -1,fromSouthOccupied = -1,fromWestOccupied = -1;
struct lock * fromNorth = NULL, * fromEast = NULL,* fromSouth = NULL,* fromWest = NULL;
struct cv * cvFromNorth = NULL, * cvFromEast = NULL, * cvFromWest = NULL,* cvFromSouth = NULL;

//make queues for cars to go in order
//struct queue *fromNorthQueue;
//struct queue *fromEastQueue;
//struct queue *fromSouthQueue;
//struct queue *fromWestQueue;
struct queue * enteringQueue[4];
struct lock * enteringLock[4];
struct cv * enteringCV[4];

/*
 * Number of cars created.
 */

#define NCARS 20


/*
 *
 * Function Definitions
 *
 */

static const char *directions[] = { "N", "E", "S", "W" };

static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s car = %2d, direction = %s, destination = %s\n",
                msgs[msg_nr], carnumber,
                directions[cardirection], directions[destdirection]);
}
 

void currentLockState(unsigned long carnumber){
    kprintf("\n\n########################\n\n");
    kprintf("Carnumber: %lu\n", carnumber);
    kprintf("NW held: %d\n", NW->held);
    kprintf("NE held: %d\n", NE->held);
    kprintf("SW held: %d\n", SW->held);
    kprintf("SE held: %d\n", SE->held);
    
    
    kprintf("\n\n########################\n\n");
}
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */
        
        (void) cardirection;
        (void) carnumber;
        
        //directions[] is char
        //passing in the index
        //currentLockState(carnumber);
        
        switch(cardirection){
            
            
            //N
            case 0:
                
                message(APPROACHING, carnumber, 0, 2);
                
                lock_acquire(SW);
                lock_acquire(NW);
                
              
                message(REGION1,  carnumber, 0, 2);
                
                
                message(REGION2,  carnumber, 0, 2);

                message(LEAVING,  carnumber, 0, 2); 

                lock_release(SW);
                lock_release(NW);
                

                break;
                
                
            //E
            case 1:            
                
                message(APPROACHING, carnumber, 1, 3);

                lock_acquire(NE);
                lock_acquire(NW);
                
                message(REGION1,  carnumber, 1, 3);
                message(REGION2,  carnumber, 1, 3);
                
                
                message(LEAVING,  carnumber, 1, 3);
                lock_release(NE);
                lock_release(NW);
                

                break;
                
                
            //S
            case 2:

                message(APPROACHING, carnumber, 2, 0);

                lock_acquire(NE);
                lock_acquire(SE);

                message(REGION1,  carnumber, 2, 0);
                message(REGION2,  carnumber, 2, 0);
                
                
                message(LEAVING,  carnumber, 2, 0);
                                                
                lock_release(NE);
                lock_release(SE);

                break;
                
                
            //W
            case 3:
                message(APPROACHING, carnumber, 3, 1);

                lock_acquire(SE);
                lock_acquire(SW);

                message(REGION1,  carnumber, 3, 1);
                message(REGION2,  carnumber, 3, 1);
                
                message(LEAVING,  carnumber, 3, 1);

                lock_release(SE);
                lock_release(SW);
                
                break;
            default:
                kprintf("ERROR: Unknown car direction\n");
                      
        }
        //currentLockState(carnumber);
        kprintf("Exit switch\n");
        
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;
        
        
        switch(cardirection){
            //N
            case 0:
                message(APPROACHING, carnumber, 0, 1);

                lock_acquire(SE);
                lock_acquire(SW);
                lock_acquire(NW);
                
                message(REGION1,  carnumber, 0, 1);
                message(REGION2,  carnumber, 0, 1);
               
                

                message(REGION3,  carnumber, 0, 1);
                message(LEAVING,  carnumber, 0, 1);
                
                
                lock_release(SE);
                lock_release(SW);
                lock_release(NW);
                
                
                break;
                
            //E
            case 1:
                message(APPROACHING, carnumber, 1, 2);
                lock_acquire(NE);
                lock_acquire(SW);
                lock_acquire(NW);
                message(REGION1,  carnumber, 1, 2);
                message(REGION2,  carnumber, 1, 2);
                message(REGION3,  carnumber, 1, 2);
                message(LEAVING,  carnumber, 1, 2);
                lock_release(NE);
                lock_release(SW);
                lock_release(NW);
                
                
                break;
            //S
            case 2:
                message(APPROACHING, carnumber, 2, 3);

                lock_acquire(NE);
                lock_acquire(SE);
                lock_acquire(NW);
                message(REGION1,  carnumber, 2, 3);
                message(REGION2,  carnumber, 2, 3);
                message(REGION3,  carnumber, 2, 3);
                message(LEAVING,  carnumber, 2, 3);
                lock_release(NE);
                lock_release(SE);
                lock_release(NW);
                break;
            //W
            case 3:
                message(APPROACHING, carnumber, 3, 0);
                lock_acquire(NE);
                lock_acquire(SE);
                lock_acquire(SW);
                message(REGION1,  carnumber, 3, 0);
                message(REGION2,  carnumber, 3, 0);
                
                
                
                message(REGION3,  carnumber, 3, 0);
                message(LEAVING,  carnumber, 3, 0);
                lock_release(NE);
                lock_release(SE);
                lock_release(SW);
                
                break;
            default:
                kprintf("ERROR: Unknown car direction\n");
                      
        }
        //currentLockState(carnumber);
        kprintf("Exit switch\n");
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;
        
        switch(cardirection){
            //N
            case 0:
                
                message(APPROACHING, carnumber, 0, 3);

                
                lock_acquire(NW);
                //n w
                message(REGION1,  carnumber, 0, 3);
                message(LEAVING,  carnumber, 0, 3);
                
                
                lock_release(NW);
                
                break;
                
            //E
            case 1:                
                message(APPROACHING, carnumber, 1, 0);

                lock_acquire(NE);
                // e n
                message(REGION1,  carnumber, 1, 0);
                message(LEAVING,  carnumber, 1, 0);
                
                
                lock_release(NE);
                break;
            //S
            case 2:
                message(APPROACHING, carnumber, 2, 1);

                lock_acquire(SE);
                
                message(REGION1,  carnumber, 2, 1);
                message(LEAVING,  carnumber, 2, 1);
                
                lock_release(SE);
                
                break;
            //W
            case 3:
                message(APPROACHING, carnumber, 3, 2);

                lock_acquire(SW);
                
                message(REGION1,  carnumber, 3, 2);
                message(LEAVING,  carnumber, 3, 2);
                
                lock_release(SW);
                
                
                break;
            default:
                kprintf("ERROR: Unknown car direction\n");
                      
        }
        
}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
        int cardirection;

        /*
         * Avoid unused variable and function warnings.
         */

        (void) unusedpointer;
        (void) carnumber;
        (void) gostraight;
        (void) turnleft;
        (void) turnright;

        cardirection = random() % 4;
        int carTurnOrStraight = random() %3;
        //lock_acquire(carsLock);
        
        //before the car enters: check if the queue is empty(so this car is head): 
        //if it is, just go, else yield for other cars 
        
        //always put it into the queue and only let the first car to arrive go
        int * firstCar;
        
        lock_acquire(enteringLock[cardirection]);
        
        //kprintf("In car direction lock\n");
        
        q_addtail(enteringQueue[cardirection],&carnumber);
                
        firstCar = q_getguy(enteringQueue[cardirection], q_getstart(enteringQueue[cardirection]));
        
        while((unsigned long)*firstCar != carnumber){//if not head
            //wait tille you are head
            cv_wait(enteringCV[cardirection], enteringLock[cardirection]);
           
            firstCar = q_getguy(enteringQueue[cardirection], q_getstart(enteringQueue[cardirection]));
            kprintf("Checking against first car: %d  I'm: %d\n", *firstCar, (int)carnumber);
        }
        
        kprintf("I'm head! %d\n", (int)*firstCar);
        //by this point this is the first car so pop off head
        q_remhead(enteringQueue[cardirection]);

        //kprintf("Leaving direction lock\n");

        //this carnumber is free to go
        switch(cardirection){
            //going from north
            case 0:
                lock_acquire(fromNorth);
                switch(carTurnOrStraight){
                    case 0:
                        gostraight(cardirection, carnumber);
                        break;

                    case 1:
                        turnleft(cardirection, carnumber);
                        
                        break;
                    case 2:
                        turnright(cardirection, carnumber);
                        break;
                    default:
                        kprintf("Error in approaching interstion\n");
                }
                lock_release(fromNorth);
                break;
            case 1:
                lock_acquire(fromEast);
                switch(carTurnOrStraight){
                    case 0:
                        gostraight(cardirection, carnumber);
                        break;
                    case 1:
                        turnleft(cardirection, carnumber);
                        break;
                    case 2:
                        turnright(cardirection, carnumber);
                        break;
                    default:
                        kprintf("Error in approaching interstion\n");
                }
                lock_release(fromEast);
                break;
            case 2:
                lock_acquire(fromSouth);

                switch(carTurnOrStraight){
                    case 0:
                        gostraight(cardirection, carnumber);
                        break;
                    case 1:
                        turnleft(cardirection, carnumber);
                        lock_release(fromSouth); 
                        break;
                    case 2:
                        turnright(cardirection, carnumber);
                        lock_release(fromSouth); 
                        break;
                    default:
                        kprintf("Error in approaching interstion\n");
                }
                lock_release(fromSouth);
                break;
            case 3:
                lock_acquire(fromWest);

                switch(carTurnOrStraight){
                    case 0:
                        gostraight(cardirection, carnumber);
                        lock_release(fromWest);
                        break;
                    case 1:
                        turnleft(cardirection, carnumber);
                        lock_release(fromWest);
                        break;
                    case 2:
                        turnright(cardirection, carnumber);
                        lock_release(fromWest);
                        break;
                    default:
                        kprintf("Error in approaching interstion\n");
                }
                lock_release(fromWest);
                break;               
        }
        lock_release(enteringLock[cardirection]);
//        lock_acquire(enteringLock[cardirection]);
//        q_remhead(enteringQueue[cardirection]);
//        lock_release(enteringLock[cardirection]);
        thread_exit();
        //lock_release(carsLock);
}


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
        int index, error;
    
        /*
         * Start NCARS approachintersection() threads.
         */

        if(NW == NULL)NW=lock_create("NW lock");
        if(NE == NULL)NE=lock_create("NE lock");
        if(SW == NULL)SW=lock_create("SW lock");
        if(SE == NULL)SE=lock_create("SE lock");
        if(fromNorth == NULL)fromNorth = lock_create("fromNorth");
        if(fromEast == NULL)fromEast = lock_create("fromEast");
        if(fromSouth == NULL)fromSouth = lock_create("fromSouth");
        if(fromWest == NULL)fromWest = lock_create("fromWest");
        //I can have at max NCARS of cars waiting at one intersection
//        fromNorthQueue = q_create(NCARS);
//        fromEastQueue = q_create(NCARS);
//        fromSouthQueue = q_create(NCARS);
//        fromWestQueue = q_create(NCARS);
        int i;
        for(i = 0; i < 4; i++){
            enteringQueue[i] = q_create(NCARS);
            enteringLock[i]=lock_create("incoming lock");
            enteringCV[i] = cv_create("incoming cv");
        }
        
        
        for (index = 0; index < NCARS; index++) {
            
                error = thread_fork("approachintersection thread",
                                    NULL, index, approachintersection, NULL);

                /*
                * panic() on error.
                */

                if (error) {         
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error));
                }
        }
        
        /*
         * wait until all other threads finish
         */

        while (thread_count() > 1)
                thread_yield();
        
        lock_destroy(NW);
        lock_destroy(NE);
        lock_destroy(SW);
        lock_destroy(SE);
        

	(void)message;
        (void)nargs;
        (void)args;
        kprintf("stoplight test done\n");
        return 0;
}

