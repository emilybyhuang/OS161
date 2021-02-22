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
	
        kprintf("In cat lock!!!!");
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
                    //take this bowl for the cat
                    whosAtBowl[0] = 0;
                    bowlAteAt = 0;
                    catmouse_eat("cat", catnumber,bowlAteAt+1, i);
                    whosAtBowl[bowlAteAt] = -1;
                    lock_release(overallLock);
                }else if(whosAtBowl[1] == -1){
                    kprintf("BOWL 2\n");
                    
                    //take this bowl for the cat
                    whosAtBowl[1] = 0;
                    bowlAteAt = 1;
                    catmouse_eat("cat", catnumber,bowlAteAt+1, i);
                    whosAtBowl[bowlAteAt] = -1;

                }else{
                    //both are taken by mouse
                    //catWaiting = cv_create("Cat waiting for lock");
                    cv_wait(catWaiting,overallLock); 
                }
            }else{
                //all cats eating
                //catWaiting = cv_create("Cat waiting for lock");
                cv_wait(catWaiting,overallLock);
            }
            
            
            //now assume the cat ate
            
            
            
            //make bowl available
            lock_acquire(overallLock);

            if(catWaiting ->isWaiting == 1){
                if(whosAtBowl[0] == -1 && whosAtBowl[1] == -1){
                    cv_broadcast(catWaiting, overallLock);
                }else{
                    //at least one available
                    cv_signal(catWaiting, overallLock);
                }
            }else{
                cv_destroy(catWaiting);	
            }
            
            
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
                    //now assume the mouse ate
                    catmouse_eat("mouse", mousenumber,1, i);
                    whosAtBowl[bowlAteAt] = -1;
                    lock_release(overallLock);
//                    cv_destroy(mouseWaiting);
                }else if(whosAtBowl[1] == -1){
                    
                    kprintf("BOWL 2\n");
                    //take this bowl for the mouse
                    whosAtBowl[1] = 1;
                    bowlAteAt = 1;
                    //now assume the mouse ate
                    catmouse_eat("mouse", mousenumber,2, i);
                    whosAtBowl[bowlAteAt] = -1;
                    lock_release(overallLock);
                    
                }else{
                    //both are taken by mouse
//                mouseWaiting = cv_create("Mouse waiting for lock");
                    cv_wait(mouseWaiting,overallLock);
                }
            }else{
//                mouseWaiting = cv_create("Mouse waiting for lock");
                cv_wait(mouseWaiting,overallLock);
            }
            
            
            
            
            
            lock_acquire(overallLock);
            if(mouseWaiting ->isWaiting == 1){
                if(whosAtBowl[0] == -1 && whosAtBowl[1] == -1){
                    cv_broadcast(mouseWaiting, overallLock);
                }else{
                    //at least one available
                    cv_signal(mouseWaiting, overallLock);
                }
            }else{
                cv_destroy(mouseWaiting);	
            }
            
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
        kprintf("IN CATMOUSE LOCK\n");
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

