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

#ifndef _PT_MUTEX_H
#define _PT_MUTEX_H

#include <pthread.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct pt_mutex {
        unsigned int count;
        pthread_t thread;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
} pt_mutex_t;

int pt_mutex_init(pt_mutex_t *mutex);

int pt_mutex_lock(pt_mutex_t *mutex);

int pt_mutex_timedlock(pt_mutex_t *mutex,const struct timespec *timeout);

int pt_mutex_trylock(pt_mutex_t *mutex);

int pt_mutex_unlock(pt_mutex_t *mutex);

int pt_mutex_destroy(pt_mutex_t *mutex);

#ifdef  __cplusplus
}
#endif

#endif /* _PT_MUTEX_H */
