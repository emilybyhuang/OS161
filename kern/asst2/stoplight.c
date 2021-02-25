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

//locks:
struct lock * NW = NULL;
struct lock * NE = NULL;
struct lock * SW = NULL;
struct lock * SE = NULL;
struct lock * carsLock = NULL;
struct cv * carsCV=NULL;

struct cv * straightNtoS = NULL, *straightStoN = NULL, *straightWtoE= NULL, *straightEtoW= NULL;
int carsInIntersection = 0;
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
int isHeld;

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
        
        
        switch(cardirection){
            //N
            case 0:
                message(APPROACHING, carnumber, 0, 2);
//                if(SW->held != isHeld || SE ->held != isHeld)lock_acquire(NW);
//                while(SW->held == isHeld && SE ->held == isHeld){
//                    cv_wait(straightNtoS, NW);
//                }
                
                
                    
                lock_acquire(NW);
//                lock_acquire(SW);
                //approching...
                carsInIntersection++;
                //n s
                message(REGION1,  carnumber, 0, 2);
                lock_acquire(SW);
                
                
                
                message(REGION2,  carnumber, 0, 2);
                lock_release(NW);
                message(LEAVING,  carnumber, 0, 2);            
                
                lock_release(SW);
                carsInIntersection--;
                break;
                
                
            //E
            case 1:                
                message(APPROACHING, carnumber, 1, 3);
//                if(NW->held != isHeld || SW ->held != isHeld)lock_acquire(NE);
//                while(NW->held == isHeld && SW ->held == isHeld){
//                    cv_wait(straightEtoW, NE);
//                }
                
                lock_acquire(NE);
                carsInIntersection++;
//                lock_acquire(NW);
                // e w
                message(REGION1,  carnumber, 1, 3);
                lock_acquire(NW);
                message(REGION2,  carnumber, 1, 3);
                lock_release(NE);
                
                message(LEAVING,  carnumber, 1, 3);
                lock_release(NW);
                carsInIntersection--;
                
                
                
                
                
                
                
                break;
                
                
            //S
            case 2:

                message(APPROACHING, carnumber, 2, 0);
//                if(NE->held != isHeld || NW ->held != isHeld)lock_acquire(SE);
//                while(NE->held == isHeld && NW ->held == isHeld){
//                    cv_wait(straightStoN, SE);
//                }
                
                lock_acquire(SE);
                carsInIntersection++;
//                lock_acquire(NE);
                //s w
                message(REGION1,  carnumber, 2, 0);
                lock_acquire(NE);
                
                
                message(REGION2,  carnumber, 2, 0);
                lock_release(SE);
                message(LEAVING,  carnumber, 2, 0);
                
                lock_release(NE);
                carsInIntersection--;
                break;
                
                
            //W
            case 3:
                message(APPROACHING, carnumber, 3, 1);
//                if(SE->held != isHeld || NE ->held != isHeld)lock_acquire(SW);
//                while(SE->held == isHeld && NE ->held == isHeld){
//                    cv_wait(straightWtoE, SW);
//                }
                carsInIntersection++;
                lock_acquire(SW);
                lock_acquire(SE);
                       
                //w e
                message(REGION1,  carnumber, 3, 1);
                lock_acquire(SE);
                
                
                message(REGION2,  carnumber, 3, 1);
                lock_release(SW);
                message(LEAVING,  carnumber, 3, 1);
                
                lock_release(SE);
                carsInIntersection--;
                break;
            default:
                kprintf("ERROR: Unknown car direction\n");
                      
        }
        
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
                lock_acquire(NW);
                
                
                //w e
                
                message(REGION1,  carnumber, 0, 1);
                lock_acquire(SW);
                lock_release(NW);
                message(REGION2,  carnumber, 0, 1);
                lock_acquire(SE);
                lock_release(SW);
                message(REGION3,  carnumber, 0, 1);
                message(LEAVING,  carnumber, 0, 1);
                
                
                lock_release(SE);
                
                
                
                break;
                
            //E
            case 1:
                                message(APPROACHING, carnumber, 1, 2);
   
                lock_acquire(NE);
                
                
//                //w e
                message(REGION1,  carnumber, 1, 2);
                lock_acquire(NW);
                lock_release(NE);
                message(REGION2,  carnumber, 1, 2);
                lock_acquire(SW);
                lock_release(NW);
                message(REGION3,  carnumber, 1, 2);
                message(LEAVING,  carnumber, 1, 2);
                
                
                lock_release(SW);
                
                
                
                break;
            //S
            case 2:
                
                                message(APPROACHING, carnumber, 1, 2);

                lock_acquire(SE);
                
                
                //w e
                message(REGION1,  carnumber, 1, 2);
                lock_acquire(NE);
                lock_release(SE);
                message(REGION2,  carnumber, 1, 2);
                lock_acquire(NW);
                lock_release(NE);
                message(REGION3,  carnumber, 1, 2);
                message(LEAVING,  carnumber, 1, 2);
                
                
                lock_release(NW);
                break;
            //W
            case 3:
                                message(APPROACHING, carnumber, 3, 0);

                lock_acquire(SW);
                
                //w e
                message(REGION1,  carnumber, 3, 0);
                lock_acquire(SE);
                lock_release(SW);
                message(REGION2,  carnumber, 3, 0);
                
                lock_acquire(NE);
                lock_release(SE);
                message(REGION3,  carnumber, 3, 0);
                message(LEAVING,  carnumber, 3, 0);
                
                
                lock_release(NE);
                
                break;
            default:
                kprintf("ERROR: Unknown car direction\n");
                      
        }
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

        /*
         * cardirection is set randomly.
         */

        cardirection = random() % 4;
        int carTurnOrStraight = random() %3;
        
        lock_acquire(carsLock);
        while(carsInIntersection>=3)cv_wait(carsCV, carsLock);
        switch(carTurnOrStraight){
            case 0:
                gostraight(cardirection, carnumber);
                break;
                
            case 1:
                //gostraight(cardirection, carnumber);
                turnright(cardirection, carnumber);
                break;
            case 2:
                //turnleft(cardirection, carnumber);
                gostraight(cardirection, carnumber);
                break;
            default:
                kprintf("Error in approaching interstion\n");
                      
                
        }
        lock_release(carsLock);
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
        isHeld= 1;

        if(NW == NULL)NW=lock_create("NW lock");
        if(NE == NULL)NE=lock_create("NE lock");
        if(SW == NULL)SW=lock_create("SW lock");
        if(SE == NULL)SE=lock_create("SE lock");
        if(carsLock == NULL)carsLock=lock_create("carsLock");
        if(straightNtoS == NULL)straightNtoS = cv_create("straightNtoS");
        if(straightStoN == NULL)straightStoN = cv_create("straightStoN");
        if(straightWtoE == NULL)straightWtoE = cv_create("straightWtoE");
        if(straightEtoW == NULL)straightEtoW = cv_create("straightEtoW");
        if(carsCV == NULL)carsCV = cv_create("carsCV");
        
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

