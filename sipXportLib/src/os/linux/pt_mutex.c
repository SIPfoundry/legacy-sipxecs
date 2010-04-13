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

/* This is the mutex implementation. */

#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include "os/linux/pt_mutex.h"

int pt_mutex_init(pt_mutex_t *mutex)
{
        mutex->count=0;
        assert(0 == (pthread_mutex_init(&mutex->mutex,NULL) | pthread_cond_init(&mutex->cond,NULL)));
        return 0;
}

int pt_mutex_lock(pt_mutex_t *mutex)
{
        int retval = 0 ;
        pthread_mutex_lock(&mutex->mutex);
        if( mutex->count && mutex->thread==pthread_self())
        {
                mutex->count++;
        }
        else
        {
           // wait for count to be 0, or error
           while(retval == 0 && mutex->count)
           {
              retval = pthread_cond_wait(&mutex->cond,&mutex->mutex);
           }
           switch ( retval )
           {
           case 0: // retval and count are 0, we now own the mutex
              mutex->count=1;
              mutex->thread=pthread_self();
              break;

           default: // all error cases
              assert(0) ;  // something is amiss
              /*NOTREACHED*/
              errno = retval;
              retval = -1;
              break;
           }
        }
        pthread_mutex_unlock(&mutex->mutex);

        return retval;
}

int pt_mutex_timedlock(pt_mutex_t *mutex,const struct timespec *timeout)
{
        int retval = 0 ;
        pthread_mutex_lock(&mutex->mutex);
        if(mutex->count && mutex->thread==pthread_self()) // allow recursive locks
        {
                mutex->count++;
        }
        else
        {
           // wait for count to be 0, or error
           while(0 == retval && mutex->count)
           {
              retval = pthread_cond_timedwait(&mutex->cond,&mutex->mutex,timeout);
           }
           switch ( retval )
           {
           case 0: // retval and count are 0, we now own the mutex
              mutex->count=1;
              mutex->thread=pthread_self();
              break;

           case ETIMEDOUT:  // timed out waiting for count to be 0
              errno = EAGAIN;
              retval = -1;
              break;

           default: // all error cases
              assert(0) ;  // if something is amiss, drop core please.
              /*NOTREACHED*/
              errno = retval;
              retval = -1;
              break;
           }
        }

        pthread_mutex_unlock(&mutex->mutex);
        return retval;
}

int pt_mutex_trylock(pt_mutex_t *mutex)
{
        int retval = 0;
        pthread_mutex_lock(&mutex->mutex);
        if(!mutex->count)
        {
                mutex->count=1;
                mutex->thread=pthread_self();
        }
        else if(mutex->thread==pthread_self())
        {
                mutex->count++;
        }
        else
        {
           errno=EAGAIN;
           retval = -1;
        }

        pthread_mutex_unlock(&mutex->mutex);
        return retval;
}

int pt_mutex_unlock(pt_mutex_t *mutex)
{
        pthread_mutex_lock(&mutex->mutex);

        if(mutex->count)
        {
                mutex->count--;
                if(!mutex->count)
                {
                   pthread_cond_broadcast(&mutex->cond);
                }
        }
        pthread_mutex_unlock(&mutex->mutex);
        return 0;
}

int pt_mutex_destroy(pt_mutex_t *mutex)
{
        if(mutex->count)
        {
                errno=EBUSY;
                return -1;
        }
        assert(0 == (pthread_mutex_destroy(&mutex->mutex) | pthread_cond_destroy(&mutex->cond)));
        return 0;
}
