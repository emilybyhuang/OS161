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
int whosAtBowl[NFOODBOWLS] = {-1};

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

char name[] = "lock";

struct lock * overallLock;

struct cv * mouseWaiting;
/*
void makeOverallLock(char * name){
	overallLock = lock_create(name);
	overallLock -> numOccupiedBowls = 0;
}
*/


int catWaitingForLock[6] = {-1};

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
	
	if(overallLock == NULL){
            overallLock = lock_create(name);
	}
	//need to have 4 meals
	int i = 0;
	while(i<NMEALS){	
            //try to grab a lock
            lock_acquire(overallLock);

            //if couldnt get a lock
            if(overallLock -> held == 1){
                    //wait for lock
                struct cv * catWaiting = cv_create("Cat waiting for lock");
                cv_wait(catWaiting,overallLock);
            //got lock
            }else{
                //check who's at the bowls
                if(whosAtBowl[0] == 1 || whosAtBowl[1] == 1){//mouse here
                    //need to wait for the mouse to be done: exit
                    struct cv * catWaiting = cv_create("Cat waiting for mouse");
                    cv_wait(catWaiting,overallLock);
                }else{//no mouse at bowls
                    //take one bowl
                    if(whosAtBowl[0] == -1){
                        whosAtBowl[0] = 0;
                    }else if(whosAtBowl[1] == -1){
                        whosAtBowl[1] = 0;
                    }	
                }
            }

            lock_release(overallLock);
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
        
        
        
        	if(overallLock == NULL){
            overallLock = lock_create(name);
	}
	//need to have 4 meals
	int i = 0;
	while(i<NMEALS){	
		//try to grab a lock
            lock_acquire(overallLock);

            //if couldnt get a lock
            if(overallLock -> held == 1){
                    //wait for lock
                struct cv * mouseWaiting = cv_create("Mouse waiting for lock");
                cv_wait(mouseWaiting,overallLock);
            //got lock
            }else{
                //check who's at the bowls
                if(whosAtBowl[0] == 0 || whosAtBowl[1] == 0){//cat here
                    //need to wait for the mouse to be done: exit
                    struct cv * mouseWaiting = cv_create("Mouse waiting for mouse");
                    cv_wait(mouseWaiting,overallLock);
                }else{//no mouse at bowls
                    //take one bowl
                    if(whosAtBowl[0] == -1){
                        whosAtBowl[0] = 1;
                    }else if(whosAtBowl[1] == -1){
                        whosAtBowl[1] = 1;
                    }	
                   
                }
            }

            lock_release(overallLock);
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
   
        /*
         * Start NCATS catlock() threads.
         */

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

        return 0;
}

