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
int whosAtBowl[NFOODBOWLS] = {-1,-1};

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
	
        
	//need to have 4 meals
	int i = 0;
	while(i<NMEALS){	
            
            //try to grab a lock
            //kprintf("1");
           
            int bowlAteAt = -1;
            
            //try to get a bowl
            
            lock_acquire(overallLock);
            //no cats eating
            
            
            while((whosAtBowl[0] == 1 || whosAtBowl[1] == 1)|| (whosAtBowl[0] != -1 && whosAtBowl[1] !=-1)){
                cv_wait(catWaiting,overallLock);
            }
            
            
            if(whosAtBowl[0] != 1 && whosAtBowl[1] != 1){
                if(whosAtBowl[0] == -1){
                    //take this bowl for the mouse
                    whosAtBowl[0] = 0;
                    bowlAteAt = 0;
                    
                }else if(whosAtBowl[1] == -1){
                    //take this bowl for the mouse
                    whosAtBowl[1] = 0;
                    bowlAteAt = 1;
                    
                }
                
            }
            
            
            //now assume the cat ate
            lock_release(overallLock);

            catmouse_eat("cat", catnumber,bowlAteAt+1, i);
            lock_acquire(overallLock);
            //make bowl available
            whosAtBowl[bowlAteAt] = -1;
            
            if(whosAtBowl[0] == -1 && whosAtBowl[1] == -1){
                cv_broadcast(mouseWaiting, overallLock); //switch to the other animal waiting
            }else{
                //at least one available
                cv_signal(catWaiting, overallLock);
            }
            
            lock_release(overallLock);
            i++;
	}
        cv_destroy(catWaiting);	
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
            //try to grab a lock
            
            int bowlAteAt = -1;
            
            //try to get a bowl
            lock_acquire(overallLock);
            
            
            // cats eating
            while((whosAtBowl[0] == 0 || whosAtBowl[1] == 0)|| (whosAtBowl[0] != -1 && whosAtBowl[1] !=-1)){
                cv_wait(mouseWaiting,overallLock);
            }
            
            
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
                    
                }
            }
            lock_release(overallLock);

            //now assume the mouse ate
            catmouse_eat("mouse", mousenumber,bowlAteAt+1, i);
            lock_acquire(overallLock);

            
            //make bowl available
            whosAtBowl[bowlAteAt] = -1;
                        
            //now can wake up any mouse waiting for this bowl
            if(whosAtBowl[0] == -1 && whosAtBowl[1] == -1){
                cv_broadcast(catWaiting, overallLock); //switch to the other animal waiting
            }else{
                cv_signal(mouseWaiting, overallLock);
            }
            
            
            lock_release(overallLock);
            i++;
	}
        cv_destroy(mouseWaiting);
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
         */
        if(catWaiting == NULL)catWaiting= cv_create("cat waiting for bowl");
        if(mouseWaiting == NULL)mouseWaiting= cv_create("Mouse waiting for bowl");
        if(overallLock == NULL)overallLock = lock_create("Lock");
	

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
        cv_destroy(catWaiting);
        cv_destroy(mouseWaiting);
        lock_destroy(overallLock);
        catWaiting = NULL;
        mouseWaiting = NULL;
        overallLock = NULL;
        kprintf("catlock test done\n");

        return 0;
}

