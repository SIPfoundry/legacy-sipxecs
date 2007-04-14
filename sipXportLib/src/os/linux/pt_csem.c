//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

/* The default LinuxThreads implementation does not have support for timing
* out while waiting for a synchronization object. Since I've already ported
* the rest of the OS dependent files to that interface, we can just drop in a
* mostly-compatible replacement written in C (like pthreads itself) that uses
* the pthread_cond_timedwait function and a mutex to build all the other
* synchronization objecs with timeout capabilities. */

/* This is the counting semaphore implementation. Binary semaphores are just
* counting semaphores with a maximum count of 1. (Incidentally, pthreads
* doesn't support maximum counts either, but as long as I'm building the
* semaphores myself I may as well include that to make life easier.) */

#include <pthread.h>
#include <errno.h>
#include <assert.h>

#include "os/linux/pt_csem.h"

int pt_sem_init(pt_sem_t *sem, unsigned int max, unsigned int count)
{
        if(!max)
                return -1;
        sem->count=count;
        sem->max=max;
        return pthread_mutex_init(&sem->mutex,NULL) | pthread_cond_init(&sem->cond,NULL);
}

int pt_sem_wait(pt_sem_t *sem)
{
        int retval = 0 ;
        pthread_mutex_lock(&sem->mutex);
        // wait for sem->count to be not zero, or error
        while(retval == 0 && !sem->count)
        {
           retval = pthread_cond_wait(&sem->cond,&sem->mutex);
        }
        switch ( retval )
        {
        case 0: // retval is 0 and sem->count is not, the sem is ours
           sem->count--;
           break ;

        default: // all error cases
           assert(0) ; // something is amiss, drop core
           /*NOTREACHED */
           errno = retval ;
           retval = -1 ;
        }
           
        pthread_mutex_unlock(&sem->mutex);

        return retval;
}

int pt_sem_timedwait(pt_sem_t *sem,const struct timespec *timeout)
{
        pthread_mutex_lock(&sem->mutex);
        int retval = 0;

        // wait for sem->count to be not zero, or error
        while (0 == retval && !sem->count)
        {
           retval = pthread_cond_timedwait(&sem->cond,&sem->mutex,timeout);
        }
        switch ( retval )
        {
        case 0: // retval is 0 and sem->count is not, the sem is ours
           sem->count--;
           break ;

        case ETIMEDOUT: // timedout waiting for count to be not zero
           errno = EAGAIN;
           retval = -1;
           break ;

        default: // all error cases
           assert(0) ; // something is amiss
           /*NOTREACHED */
           errno = retval ;
           retval = -1 ;
        }
        pthread_mutex_unlock(&sem->mutex);
        return retval;
}

int pt_sem_trywait(pt_sem_t *sem)
{
        pthread_mutex_lock(&sem->mutex);
        if(sem->count)
        {
                sem->count--;
                pthread_mutex_unlock(&sem->mutex);
                return 0;
        }
        errno=EAGAIN;
        pthread_mutex_unlock(&sem->mutex);
        return -1;
}

int pt_sem_post(pt_sem_t *sem)
{
        pthread_mutex_lock(&sem->mutex);
        if(sem->count<sem->max)
        {
                sem->count++;
                pthread_cond_broadcast(&sem->cond);
                pthread_mutex_unlock(&sem->mutex);
                return 0;
        }
        errno=ERANGE;
        pthread_mutex_unlock(&sem->mutex);
        return -1;
}

int pt_sem_getvalue(pt_sem_t *sem)
{
        return sem->count;
}

int pt_sem_destroy(pt_sem_t *sem)
{
        return pthread_mutex_destroy(&sem->mutex) | pthread_cond_destroy(&sem->cond);
}
