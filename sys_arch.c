/**
 * Architecture wrapping functions for LWIP
 */

#include <arch/sys_arch.h>

#if NO_SYS

#include <hardware/systimer.h>

u32_t sys_now(void)
{
	return timerGetAbsoluteTime();
}

#else // NO_SYS
#include <lwip/sys.h>
#include <kernel/malloc.h>
#include <hardware/systimer.h>

static ATOM_TCB *last_tcb;

//============================================================================

err_t sys_mutex_new(sys_mutex_t *mutex)
{
	atomMutexCreate(mutex);
	return 0;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
	atomMutexGet(mutex, 0);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
	atomMutexPut(mutex);
}

void sys_mutex_free(sys_mutex_t *mutex)
{
	atomMutexDelete(mutex);
	mutex->count = 0xFF;
}

/** Check if a mutex is valid/allocated: return 1 for valid, 0 for invalid */
int sys_mutex_valid(sys_mutex_t *mutex)
{
	return mutex->count!=0xFF;
}

/** Set a mutex invalid so that sys_mutex_valid returns 0 */
void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
	mutex->count = 0xFF;
}

//============================================================================
static int32_t convertTimeout(u32_t timeout)
{
	if (timeout==0)
		return 0;
	if (timeout < (1000/SYSTEM_TICKS_PER_SEC))
		return 1;
	return timeout * SYSTEM_TICKS_PER_SEC / 1000;
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	kassert(count!=0xFF);
	atomSemCreate(sem, count);
	return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{
	atomSemDelete(sem);
}

void sys_sem_signal(sys_sem_t *sem)
{
	atomSemPut(sem);
}

/**
 * Acquire semaphore.
 * \param  timeout Wait timeout in ms (0=forever)
 * \return Time waited in ms (0=semaphore was immediately available)
 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	int32_t waited = 0xDEAD;
	uint8_t result;
	int32_t to = convertTimeout(timeout);

	int32_t start = timerGetAbsoluteTime();
	result = atomSemGetTimed(sem, to, &waited);
	kassert(waited!=0xDEAD);
	if (result==ATOM_TIMEOUT)
		return SYS_ARCH_TIMEOUT;
	kassert(result==ATOM_OK);
	return timerGetAbsoluteTime() - start;
}

int sys_sem_valid(sys_sem_t *sem)
{
	return sem->count!=0xFF;
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
	sem->count = 0xFF;
}

//============================================================================

/**
 * Allocate queue of void*.
 */
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	uint8_t *buf;

	kassert(size > 0); // this can fail if one of the xxx_MBOX_SIZE defines hasn't been set.
	buf = kalloc(size*sizeof(void*));
	if (buf==NULL)
		return SYS_MBOX_NULL;

	atomQueueCreate(mbox, buf, sizeof(void*), size);
	return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
	atomQueueDelete(mbox);
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	atomQueuePut(mbox, 0, (uint8_t*)&msg);
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	uint8_t result;
	int32_t waited=0xDEAD;
	void *recvMsg;
	int32_t to = convertTimeout(timeout);
	
	int32_t start = timerGetAbsoluteTime();
	result = atomQueueGetTimed(mbox, to, (uint8_t*)&recvMsg, &waited);
	if (result==ATOM_TIMEOUT) {
		return SYS_ARCH_TIMEOUT;
	} else {
		kassert(result==ATOM_OK);
		if (msg!=NULL)
			*msg = recvMsg;
		return timerGetAbsoluteTime() - start;
	}
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	uint8_t result;
	
	result = atomQueueGet(mbox, -1, (uint8_t*)msg);
	if (result==ATOM_WOULDBLOCK)
		return SYS_MBOX_EMPTY;
	return 0;
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	uint8_t result;
	
	result = atomQueuePut(mbox, -1, (uint8_t*)&msg);
	if (result==ATOM_WOULDBLOCK)
		return ERR_MEM;
	return ERR_OK;
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
	return mbox->unit_size==sizeof(void*);
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	mbox->unit_size = 0xFF;
}

//============================================================================

/// Data needed for starting an LWIP thread
struct sThreadData {
	ATOM_TCB tcb;

	void (*entryFunction)(void *arg);
	void *entryArg;

	uint8_t stack_bottom;
};

#if 0
struct sys_timeouts* sys_arch_timeouts(void)
{
	void *timeouts = atomGetSpecific(0);
	if (timeouts==NULL) {
		// no timeouts structure has been set for the current thread, create one
		// this will happen for threads that have not been created by sys_thread_new()
		// but need to use an LWIP timeout, e.g. the main thread.
		timeouts = kalloc(sizeof(struct sys_timeouts));
		osAssert(timeouts!=NULL);
		memset(timeouts, 0, sizeof(struct sys_timeouts));
		atomSetSpecific(0, timeouts);
	}
	return (struct sys_timeouts*)timeouts;
}
#endif

/**
 * Wraps up the main function of a LWIP thread to convert the calling parameter
 * and set up the thread-specific timeout structure.
 */
static void lwipThreadWrapper(uint32_t param)
{
	struct sThreadData *threadData;

	threadData = (struct sThreadData*)param;
	threadData->entryFunction(threadData->entryArg);
}

sys_thread_t sys_thread_new(const char *name, void (* thread)(void *arg), void *arg, int stacksize, int prio)
{
	struct sThreadData *data;

	// allocate
	if ((data = kalloc(sizeof(struct sThreadData)+stacksize))==NULL)
		goto out;

	data->entryFunction = thread;
	data->entryArg = arg;

	last_tcb = &data->tcb;

	if (atomThreadCreate(&data->tcb, prio, lwipThreadWrapper, (uint32_t)data, 
				(&data->stack_bottom) + stacksize, stacksize)!=ATOM_OK)
		goto out1;

	return &data->tcb;

out1:
	kfree(data);
out:
	return NULL;
}

//============================================================================

void sys_init(void)
{
}

#endif // NO_SYS
