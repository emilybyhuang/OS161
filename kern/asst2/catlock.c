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
struct lock * bowl1 = NULL;
struct lock * bowl2 = NULL;

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

        
        
	//need to have 4 meals
	int i = 0;
	while(i<NMEALS){	
            
            //try to grab a lock
            //kprintf("1");
            //kprintf("2");
            
            int bowlAteAt = -1;
//            int ate = 0;
            
            //try to get a bowl
            
            lock_acquire(overallLock);
            //no mouse eating
            while((whosAtBowl[0] == 1 || whosAtBowl[1] == 1) || (whosAtBowl[0] != -1 && whosAtBowl[1] != -1)){
                cv_wait(catWaiting,overallLock);
            }
        
                
                
            //take whatever bowl's available
            if(whosAtBowl[0] == -1){
                
                //take this bowl for the cat
                whosAtBowl[0] = 0;
                bowlAteAt = 0;
//                ate = 1;
                lock_release(overallLock);//other animals can now check the bowl status but they just can't come into the eating bowl
                catmouse_eat("cat", catnumber,1, i);
                //lock_acquire(bowl1);
                lock_acquire(overallLock);

                whosAtBowl[bowlAteAt] = -1;
                if(mouseWaiting -> isWaiting)cv_broadcast(mouseWaiting, overallLock);
                //lock_release(bowl1);
                lock_release(overallLock);

            }else if(whosAtBowl[1] == -1){
                kprintf("BOWL 2\n");
                
                //take this bowl for the cat
                whosAtBowl[1] = 0;
                bowlAteAt = 1;
//                ate =1;
                lock_release(overallLock);
                catmouse_eat("cat", catnumber,2, i);
//                lock_acquire(bowl2);
                lock_acquire(overallLock);
                whosAtBowl[bowlAteAt] = -1;
                if(mouseWaiting -> isWaiting)cv_broadcast(mouseWaiting, overallLock);
                lock_release(overallLock);
            }else{
                cv_wait(catWaiting, overallLock);
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
            //try to grab a lock
//            int ate = 0;
            int bowlAteAt = -1;
            
            //try to get a bowl
            lock_acquire(overallLock);
            
            //min one cat or full
            while((whosAtBowl[0] == 0 || whosAtBowl[1] == 0) || (whosAtBowl[0] != -1 && whosAtBowl[1] != -1)){
                cv_wait(mouseWaiting,overallLock);
            }

            
            if(whosAtBowl[0] == -1){
                whosAtBowl[0] = 1;
                bowlAteAt = 0;
                lock_release(overallLock);
                catmouse_eat("mouse", mousenumber,1, i);
                
                lock_acquire(overallLock);
                whosAtBowl[bowlAteAt] = -1;
                if(catWaiting -> isWaiting)cv_broadcast(catWaiting, overallLock);
                lock_release(overallLock);
            }else if(whosAtBowl[1] == -1){
                whosAtBowl[1] = 1;
                bowlAteAt = 1;
                lock_release(overallLock);
                catmouse_eat("mouse", mousenumber,2, i);
                
                lock_acquire(overallLock);
                whosAtBowl[bowlAteAt] = -1;
                if(catWaiting -> isWaiting)cv_broadcast(catWaiting, overallLock);
                lock_release(overallLock);
            }else{
                cv_wait(mouseWaiting, overallLock);
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
        if(bowl1 == NULL) lock_create("bowl1");
        if(bowl2 == NULL) lock_create("bowl2");
        if(overallLock == NULL)overallLock = lock_create(name);
        if(overallLock == NULL)overallLock = lock_create(name);
        if(mouseWaiting == NULL)mouseWaiting= cv_create("Mouse waiting for bowl");
        if(catWaiting == NULL)catWaiting= cv_create("cat waiting for bowl");


        
        
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
        lock_destroy(bowl1);
        lock_destroy(bowl2);
        lock_destroy(overallLock);
        cv_destroy(mouseWaiting);
        cv_destroy(catWaiting);



        return 0;
}

