/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	assert(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);

	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}
	
	// add stuff here as needed
	// initialize the lock
	//lock -> numOccupiedBowls = 0;
        lock -> held = 0;//nothing is taking this lock yet
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	// add stuff here as needed
	
	kfree(lock->name);
	kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	// Write this
	// check the lock first
	if(lock == NULL)panic("This lock ptr is pointing to null");

	int spl = splhigh();
        
        int lockInitialState = lock -> held;
        //try to get lock
        if(lockInitialState == 0){
            lock -> held = 1;
        }else if(lockInitialState == 1){
            while(lock -> held  == 1){
                thread_sleep(lock);//sleep on this lock 
            }
            lock -> held = 1;//woke up and get lock now
        }
        
	//enable interrupts
	splx(spl);

	(void)lock;  // suppress warning until code gets written
}

void
lock_release(struct lock *lock)
{
	// Write this

	if(!lock)panic("This lock ptr is pointing to null");
	
	int spl = splhigh();

	//make no thread occupy this lock
        lock -> held = 0;
	
	//wakeup whatever that was waiting for this lock
	thread_wakeup(lock);	

	//enable interrupts
	splx(spl);

	(void)lock;  // suppress warning until code gets written
}

//check if the lock passed in is holding the current thread
int
lock_do_i_hold(struct lock *lock)
{
	// Write this
	int numToReturn = 0;
	//if current thread is the same as the lock thread
        if(lock -> held == 1)numToReturn = 1;
        else numToReturn = 0;

	(void)lock;  // suppress warning until code gets written

	return numToReturn;    // dummy until code gets written
}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	
	// add stuff here as needed
	cv -> isWaiting = 0;
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	// add stuff here as needed
	
	kfree(cv->name);
	kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written

	// Write this
        if(cv == NULL || lock == NULL)panic("Null parameters passed in!\n");
	
	// Release the supplied lock
	if(lock_do_i_hold(lock))lock_release(lock);
        
        //go back to interrupt disable
        int spl = splhigh();
        
        cv -> isWaiting = 1;
        
	// go to sleep
	thread_sleep(cv);

	// reenable interrpupts
	splx(spl);
	
	// after waking up: reaquire lock
	lock_acquire(lock);

}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	// Write this
	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written

	//wake up one thread sleeping on this
	
	//disable interrupt
	int spl = splhigh();

	//wake up the first thread in the waiting list
	//check if the current thread has the lock 
        cv -> isWaiting = 0;
	if(lock_do_i_hold(lock) == 1)thread_wakeup_one(cv);
	
	//enable interrupt
	splx(spl);
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	// Write this
	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written

	//disable interrupt
	int spl= splhigh();

	//wakes up everything sleeping on cv
        cv -> isWaiting = 0;
	if(lock_do_i_hold(lock)==1)thread_wakeup(cv);
	//enable interrupts again
	splx(spl);

}
