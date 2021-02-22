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
struct cv * catWaiting = NULL;
struct cv * mouseWaiting = NULL;

//struct cv * mouseWaiting;
//struct cv * catWaiting;
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
	
        kprintf("1");
	if(overallLock == NULL){
            overallLock = lock_create(name);
	}
        
        
	//need to have 4 meals
	int i = 0;
	while(i<NMEALS){	
            if(catWaiting == NULL)catWaiting= cv_create("cat waiting for bowl");
            //try to grab a lock
            //kprintf("1");
            kprintf("2");
            
            int bowlAteAt = -1;
            
            //try to get a bowl
            
            lock_acquire(overallLock);
            //no cats eating
            if(whosAtBowl[0] != 1 && whosAtBowl[1] != 1){
                if(whosAtBowl[0] == -1){
                            kprintf("3");
                    //take this bowl for the mouse
                    whosAtBowl[0] = 0;
                    bowlAteAt = 0;
                    
                }else if(whosAtBowl[1] == -1){
                            kprintf("4");
                    //take this bowl for the mouse
                    whosAtBowl[1] = 0;
                    bowlAteAt = 1;
                    
                }else{
                            kprintf("5");
                    //both are taken by mouse
                    //catWaiting = cv_create("Cat waiting for lock");
                    
                    cv_wait(catWaiting,overallLock);
                   
                }
            }else{
                        kprintf("6");
                //all cats eating
                //catWaiting = cv_create("Cat waiting for lock");
                cv_wait(catWaiting,overallLock);
            }
            
            
            //now assume the mouse ate
            //make bowl available
            whosAtBowl[bowlAteAt] = -1;
            
            if(whosAtBowl[0] == -1 && whosAtBowl[1] == -1){
                cv_broadcast(catWaiting, overallLock);
            }else{
                //at least one available
                cv_signal(catWaiting, overallLock);
            }
            cv_destroy(catWaiting);
            lock_release(overallLock);
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
        
        
        if(overallLock == NULL){
            overallLock = lock_create(name);
	}
        
        if(mouseWaiting == NULL)mouseWaiting= cv_create("Mouse waiting for bowl");
	//need to have 4 meals
	int i = 0;
	while(i<NMEALS){	
            //try to grab a lock
            
            int bowlAteAt = -1;
            
            //try to get a bowl
            lock_acquire(overallLock);
            //no cats eating
            if(whosAtBowl[0] != 0 && whosAtBowl[1] != 0){
                if(whosAtBowl[0] == -1){
                    //take this bowl for the mouse
                    whosAtBowl[0] = 1;
                    bowlAteAt = 0;
//                    cv_destroy(mouseWaiting);
                }else if(whosAtBowl[1] == -1){
                    //take this bowl for the mouse
                    whosAtBowl[1] = 1;
                    bowlAteAt = 1;
                    
                }else{
                    //both are taken by mouse
//                mouseWaiting = cv_create("Mouse waiting for lock");
                    cv_wait(mouseWaiting,overallLock);
                }
            }else{
//                mouseWaiting = cv_create("Mouse waiting for lock");
                cv_wait(mouseWaiting,overallLock);
            }
            
            
            //now assume the mouse ate
            //make bowl available
            whosAtBowl[bowlAteAt] = -1;
            
            //now can wake up any mouse waiting for this bowl
            if(whosAtBowl[0] == -1 && whosAtBowl[1] == -1){
                cv_broadcast(mouseWaiting, overallLock);
            }else{
                cv_signal(mouseWaiting, overallLock);
            }
            
            cv_destroy(mouseWaiting);
            lock_release(overallLock);
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

