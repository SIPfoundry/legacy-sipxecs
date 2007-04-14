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

#ifndef _PT_CSEM_H
#define _PT_CSEM_H

#include <pthread.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct pt_sem {
        unsigned int count;
        unsigned int max;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
} pt_sem_t;

int pt_sem_init(pt_sem_t *sem, unsigned int max, unsigned int count);

int pt_sem_wait(pt_sem_t *sem);

int pt_sem_timedwait(pt_sem_t *sem,const struct timespec *timeout);

int pt_sem_trywait(pt_sem_t *sem);

int pt_sem_post(pt_sem_t *sem);

int pt_sem_getvalue(pt_sem_t *sem);

int pt_sem_destroy(pt_sem_t *sem);

#ifdef  __cplusplus
}
#endif

#endif /* _PT_CSEM_H */
