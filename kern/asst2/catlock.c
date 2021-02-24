//Key points: 
//    1.there will be one general lock that takes care of the bowls: so that you 
//    only modify the bowl status
//        
//

/*
 * catlock.c
 *
 * Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
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
#include "catmouse.h"

//if the element == 0: cat occupying it
//if element == 1: mouse occupying it
int whosAtBowl[NFOODBOWLS] = {-1, -1};

/*
 * 
 * Function Definitions
 * 
 */

/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */


struct lock * overallLock = NULL;
struct cv * catWaiting = NULL;
struct cv * mouseWaiting = NULL;
struct lock * bowl1 = NULL;
struct lock * bowl2 = NULL;

//has to be general since you might have a bias
struct cv * waitingForDish = NULL;


//struct cv * mouseWaiting;
//struct cv * catWaiting;
/*
void makeOverallLock(char * name){
	overallLock = lock_create(name);
	overallLock -> numOccupiedBowls = 0;
}
*/



static
void
catlock(void * unusedpointer, 
        unsigned long catnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) catnumber;
        

	

        int i = 0;
	while(i<NMEALS){	
            
            //try to grab a lock            
            kprintf("\n!cat trying...\n");
            //try to get a bowl
            lock_acquire(overallLock);
            //no mouse eating
                        kprintf("occuppied ele: whosAtBowl[0]: %d   whosAtBowl[1]: %d\n", whosAtBowl[0], whosAtBowl[1]);

            
            int mouseEating = (whosAtBowl[0] == 1) || (whosAtBowl[1] == 1);
            kprintf("1. Mouse eating: %d\n",mouseEating );
            int fullyOccupied = (whosAtBowl[0] != -1 )&& (whosAtBowl[1] != -1);
            kprintf("2. ullyOccupied: %d\n",fullyOccupied );
            
            while( mouseEating || fullyOccupied){
                cv_wait(waitingForDish,overallLock);
            }
            
            kprintf("cat ok!\n");
               
            //now sure the cat can eat!
            
            //take whatever bowl is available
            if(whosAtBowl[0] == -1){
                
                //take this bowl for the cat
                whosAtBowl[0] = 0;
                lock_release(overallLock);
                
                //other animals can now check the bowl status, 
                //say to take the other bowl if available but they just can't come into the eating bowl
                //lock_acquire(bowl1);
                catmouse_eat("cat", catnumber,1, i);
                
                //done eating so reset status to avail
                lock_acquire(overallLock);
                whosAtBowl[0] = -1;
                cv_broadcast(waitingForDish, overallLock);
                //lock_release(bowl1);
                lock_release(overallLock);

            }else if(whosAtBowl[1] == -1){
                
                //take this bowl for the cat
                whosAtBowl[1] = 0;
                lock_release(overallLock);
                
                //other animals can now check the bowl status, 
                //say to take the other bowl if available but they just can't come into the eating bowl
                                //lock_acquire(bowl2);

                catmouse_eat("cat", catnumber,2, i);
                
                //done eating so reset status to avail
                lock_acquire(overallLock);
                whosAtBowl[1] = -1;
                cv_broadcast(waitingForDish, overallLock);
//                                lock_release(bowl2);

                lock_release(overallLock);
            }else{
                kprintf("ERROR in catlock\n");
                //lock_release(overallLock);
            }

            i++;
	}
        
}
	

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(void * unusedpointer,
          unsigned long mousenumber)
{
        /*
         * Avoid unused variable warnings.
         */
        
        (void) unusedpointer;
        (void) mousenumber;
        
      
        
	//need to have 4 meals
	int i = 0;
	while(i<NMEALS){	
            kprintf("\nmouse trying...\n");
            kprintf("occuppied ele: whosAtBowl[0]: %d   whosAtBowl[1]: %d\n", whosAtBowl[0], whosAtBowl[1]);

            //try to get a bowl
            lock_acquire(overallLock);
            
            //min one cat or full
            int catEating = (whosAtBowl[0] == 0) || (whosAtBowl[1] == 0);
            kprintf("1. cat eating: %d\n",catEating );
            int fullyOccupied = (whosAtBowl[0] != -1 )&& (whosAtBowl[1] != -1);
            kprintf("2. ullyOccupied: %d\n",fullyOccupied );
            

            while( catEating || fullyOccupied ){
                cv_wait(waitingForDish,overallLock);
            }
            

            
            kprintf("mouse!\n");
            
            if(whosAtBowl[0] == -1){
                //take this bowl for the mouse
                whosAtBowl[0] = 1;
                //now assume the mouse ate
                lock_release(overallLock);
//                lock_acquire(bowl1);

                
                catmouse_eat("mouse", mousenumber,1, i);
                
                //now doing eating: the bowl status will change
                lock_acquire(overallLock);

                whosAtBowl[0] = -1;
                
                cv_broadcast(waitingForDish, overallLock);
                lock_release(overallLock);
//                lock_release(bowl1);
                
            }else if(whosAtBowl[1] == -1){

                //take this bowl for the mouse
                whosAtBowl[1] = 1;
                //now assume the mouse ate
                lock_release(overallLock);
//                                lock_acquire(bowl2);

                
                catmouse_eat("mouse", mousenumber,2, i);
                
                lock_acquire(overallLock);
                
                whosAtBowl[1] = -1;
                cv_broadcast(waitingForDish, overallLock);
                lock_release(overallLock);
//                                lock_release(bowl2);

                
            }else{
                kprintf("ERROR in mouselock\n");
                //lock_release(overallLock);
                //cv_wait(waitingForDish,overallLock);
            }
            i++;
	}
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
             char ** args)
{
        int index, error;
        kprintf("IN CATMOUSE LOCK\n");
        /*
         * Start NCATS catlock() threads.
         * 
         */
        if(overallLock == NULL)overallLock = lock_create("Lock");
        if(mouseWaiting == NULL)mouseWaiting= cv_create("Mouse waiting for bowl");
        if(catWaiting == NULL)catWaiting= cv_create("Cat waiting for bowl");
        if(waitingForDish == NULL) waitingForDish = cv_create("Waiting for bowl");
        if(bowl1 == NULL )bowl1 = lock_create("bowl 1");
                if(bowl2 == NULL )bowl1 = lock_create("bowl 2");



        
        
        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catlock thread", 
                                    NULL, 
                                    index, 
                                    catlock, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catlock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * Start NMICE mouselock() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mouselock thread", 
                                    NULL, 
                                    index, 
                                    mouselock, 
                                    NULL
                                    );
      
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mouselock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
        
        /*
         * wait until all other threads finish
         */
        
        while (thread_count() > 1)
                thread_yield();

        (void)nargs;
        (void)args;
        kprintf("catlock test done\n");
        
        
        //delete all the pointers

        lock_destroy(overallLock);
        cv_destroy(mouseWaiting);
        cv_destroy(catWaiting);
        cv_destroy(waitingForDish);
        
        overallLock = NULL;
        catWaiting = NULL;
        mouseWaiting = NULL;
        waitingForDish = NULL;
        lock_destroy(bowl1);

        lock_destroy(bowl2);
        bowl1 = NULL;
        bowl2 = NULL;


        return 0;
}

