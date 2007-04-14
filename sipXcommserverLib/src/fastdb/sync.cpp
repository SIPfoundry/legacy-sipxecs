//-< SYNC.CPP >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Intertask synchonization primitives
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "stdtp.h"
#include "sync.h"
#include "os/OsSysLog.h"

#ifndef _WIN32

unsigned dbSystem::getCurrentTimeMsec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec / 1000;
}


#ifndef USE_POSIX_API

#include <errno.h>
#include <signal.h>

#define PRINT_ERROR(func)  perror(func)

char const*  keyFileDir = SIPX_TMPDIR "/";

static void alarm_handler(int){}

class moduleInitializer {
  public:   
    moduleInitializer() { 
        static struct sigaction sigact; 
	sigact.sa_handler = alarm_handler;
	::sigaction(SIGALRM, &sigact, NULL);
    }
};

static moduleInitializer initializer; // install SIGLARM handler



bool dbSharedMemory::open(char const* name, size_t size)
{
    char* fileName = (char*)name;
    if (strchr(name, '/') == NULL) { 
	fileName = new char[strlen(name)+strlen(keyFileDir)+1];
	sprintf(fileName, "%s%s", keyFileDir, name);
    }
    int fd = ::open(fileName, O_RDWR|O_CREAT, 0777);
    if (fd < 0) { 
	if (fileName != name) { 
	    delete[] fileName;
	}
	return false;
    } 
    ::close(fd);
    int key = ftok(fileName, '0');
    if (fileName != name) { 
	delete[] fileName;
    }
    if (key < 0) { 
	return false;
    }
    shm = shmget(key, DOALIGN(size, 4096), IPC_CREAT|0777);
    if (shm < 0) { 
	return false;
    }
    ptr = (char*)shmat(shm, NULL, 0);
    return (ptr != (char*)-1);
}

void dbSharedMemory::close()
{
    shmdt((char*)ptr);
}

void dbSharedMemory::erase()
{
    close();
    shmctl(shm, IPC_RMID, NULL);
}
   

int sem_init(int& sem, char const* name, unsigned init_value)
{
    key_t key = IPC_PRIVATE;
    int semid;
    struct sembuf sops[3];
    sops[0].sem_num = 1;
    sops[0].sem_op  = 0; /* check if semaphore was already initialized */
    sops[0].sem_flg = IPC_NOWAIT;
    sops[1].sem_num = 1;
    sops[1].sem_op  = 1; /* mark semaphore as initialized */
    sops[1].sem_flg = 0;
    sops[2].sem_num = 0;
    sops[2].sem_op  = init_value;
    sops[2].sem_flg = 0;
    if (name != NULL) { 
	int fd;
	char* path = (char*)name;
	if (strchr(name, '/') == NULL) { 
	    path = new char[strlen(name)+strlen(keyFileDir)+1];
	    sprintf(path, "%s%s", keyFileDir, name);
	}
	fd = open(path, O_WRONLY|O_CREAT, 0777);
	if (fd < 0) {
	    PRINT_ERROR("open");
            OsSysLog::add(FAC_DB, PRI_CRIT,
                          "Error attempting to open '%s' for writing.",
                          path);
	    if (path != name) { 
		delete[] path;
	    }
	    return -1;
	}
	close(fd);
	key = ftok(path, '0');
        OsSysLog::add(FAC_DB, PRI_DEBUG,
                      "sem_init path = '%s', key = 0x%x",
                      path, key);
	if (key < 0) {
	    PRINT_ERROR("ftok");
            fprintf(stderr, "Error calling ftok('%s').\n", path);
	    if (path != name) { 
	      delete[] path;
	    }
	    return -1;
	}
	if (path != name) { 
	    delete[] path;
	}
    }
    OsSysLog::add(FAC_DB, PRI_DEBUG,
                  "sem_init semget(0x%x, 2, IPC_CREAT|0777)",
                  key);
    semid = semget(key, 2, IPC_CREAT|0777);
    if (semid < 0) { 
	PRINT_ERROR("semget");
        OsSysLog::add(FAC_DB, PRI_CRIT,
                      "sem_init semget failed: key = 0x%x",
                      key);
	return -1;
    }
    if (semop(semid, sops, itemsof(sops)) && errno != EAGAIN) { 
	PRINT_ERROR("semop");
	return -1;
    }
    sem = semid;
    return 0;
}

enum wait_status { wait_ok, wait_timeout_expired, wait_error };

static wait_status wait_semaphore(int& sem, unsigned msec, 
				  struct sembuf* sops, int n_sops)
{
    if (msec != INFINITE) { 
	struct timeval start;
	struct timeval stop;
	gettimeofday(&start, NULL);
	unsigned long usec = start.tv_usec + msec % 1000 * 1000;
	stop.tv_usec = usec % 1000000;
	stop.tv_sec = start.tv_sec + msec / 1000 + usec / 1000000;

	while (true) { 
	    struct itimerval it;
	    it.it_interval.tv_sec = 0;
	    it.it_interval.tv_usec = 0;
	    it.it_value.tv_sec = stop.tv_sec - start.tv_sec;
	    it.it_value.tv_usec = stop.tv_usec - start.tv_usec;
	    if (stop.tv_usec < start.tv_usec) { 
		it.it_value.tv_usec += 1000000;
		it.it_value.tv_sec -= 1;
	    }
	    if (setitimer(ITIMER_REAL, &it, NULL) < 0) { 
		return wait_error;
	    }
	    if (semop(sem, sops, n_sops) == 0) { 
		break;
	    }
	    if (errno != EINTR) { 
		return wait_error;
	    }
	    gettimeofday(&start, NULL);
	    if (stop.tv_sec < start.tv_sec || 
	       (stop.tv_sec == start.tv_sec && stop.tv_usec < start.tv_sec))
	    {
		return wait_timeout_expired;
	    }
	}
    } else { 
	while (semop(sem, sops, n_sops) < 0) { 
	    if (errno != EINTR) { 
		return wait_error;
	    }
	}
    }
    return wait_ok;
}


bool dbSemaphore::wait(unsigned msec)
{
    static struct sembuf sops[] = {{0, -1, 0}};
    wait_status ws = wait_semaphore(s, msec, sops, itemsof(sops));
    assert(ws != wait_error);
    return ws == wait_ok;
}

void dbSemaphore::signal(unsigned inc)
{
    if (inc != 0) { 
	struct sembuf sops[1];
	sops[0].sem_num = 0;
	sops[0].sem_op  = inc;
	sops[0].sem_flg = 0;
	int rc = semop(s, sops, 1);
	assert(rc == 0); 
    }
}

#if (defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)) || defined(__FreeBSD__) 
/* union semun is defined by including <sys/sem.h> */  
#else
union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};
#endif
static union semun u;


void dbSemaphore::reset() 
{
    u.val = 0;
    int rc = semctl(s, 0, SETVAL, u);
    assert(rc >= 0);
}

bool dbSemaphore::open(char const* name, unsigned init_value)
{
    return sem_init(s, name, init_value) == 0;
}

void dbSemaphore::close() {}

void dbSemaphore::erase() {
    semctl(s, 0, IPC_RMID, &u);
}

bool dbEvent::wait(unsigned msec)
{
    static struct sembuf sops[] = {{0, -1, 0}, {0, 1, 0}};
    wait_status ws = wait_semaphore(e, msec, sops, itemsof(sops));
    assert(ws != wait_error);
    return ws == wait_ok;
}

void dbEvent::signal()
{
    static struct sembuf sops[] = {{0, 0, IPC_NOWAIT}, {0, 1, 0}};
    int rc = semop(e, sops, itemsof(sops));
    assert(rc == 0 || errno == EAGAIN); 
}

void dbEvent::reset()
{
    static struct sembuf sops[] = {{0, -1, IPC_NOWAIT}};
    int rc = semop(e, sops, itemsof(sops));
    assert(rc == 0 || errno == EAGAIN); 
}

bool dbEvent::open(char const* name, bool signaled)
{
    return sem_init(e, name, signaled) == 0;
}

void dbEvent::close() {}

void dbEvent::erase() {
    semctl(e, 0, IPC_RMID, &u);
}





#if !defined(__osf__) && !defined(__sun)

// 7/28/03 (rschaaf):
// The following cleverness does not work.  It can lead to
// unbalanced SEM_UNDO counts for a process.  If this happens,
// when the process exits, the semaphore value will be modified
// to an incorrect value which can either prevent any process from
// entering the critical section or can alternatively allow more
// than one thread/process in the critical section at the same time.
#if defined(__GNUC__) && defined(i386) && defined(THIS_DOESNT_WORK)

void dbGlobalCriticalSection::enter()
{
    int inc = -1;
    __asm__ __volatile__(
			"lock; xadd %0,%1"
			:"=d" (inc), "=m" (*count)
			:"d" (inc), "m" (*count));
    if (inc != 1) { 
	static struct sembuf sops[] = {{0, -1, SEM_UNDO}};
	int rc;
	while ((rc = semop(semid, sops, 1)) < 0 && errno == EINTR);
	assert(rc == 0);
    }				       
}

void dbGlobalCriticalSection::leave()
{
    int inc = 1;
    __asm__ __volatile__(
			"lock; xadd %0,%1"
			:"=d" (inc), "=m" (*count)
			:"d" (inc), "m" (*count));
    if (inc != 0) { 
	/* some other processes waiting to enter critical section */
	static struct sembuf sops[] = {{0, 1, SEM_UNDO}};
	int rc = semop(semid, sops, 1);
	assert(rc == 0);
    }
}

bool dbGlobalCriticalSection::create(char const* name, sharedsem_t* count)
{
    this->count = count;
    *count = 1;
    return sem_init(semid, name, 0) == 0;
}

bool dbGlobalCriticalSection::open(char const* name, sharedsem_t* count)
{
    this->count = count;
    return sem_init(semid, name, 0) == 0;
}

#else // defined(__GNUC__) && defined(i386) && defined(THIS_DOESNT_WORK)

void dbGlobalCriticalSection::enter()
{
    static struct sembuf sops[] = {{0, -1, SEM_UNDO}};
    int rc;
    while ((rc = semop(semid, sops, 1)) < 0 && errno == EINTR);
    assert(rc == 0);
}

void dbGlobalCriticalSection::leave()
{
    static struct sembuf sops[] = {{0, 1, SEM_UNDO}};
    int rc = semop(semid, sops, 1);
    assert(rc == 0);
}

bool dbGlobalCriticalSection::open(char const* name, sharedsem_t*)
{
    return sem_init(semid, name, 1) == 0;
}

bool dbGlobalCriticalSection::create(char const* name, sharedsem_t*)
{
    return sem_init(semid, name, 1) == 0;
}

#endif // defined(__GNUC__) && defined(i386) && defined(THIS_DOESNT_WORK)

void dbGlobalCriticalSection::erase()
{
    semctl(semid, 0, IPC_RMID, &u);
}

#endif // !defined(__osf__) && !defined(__sun)
	


dbInitializationMutex::initializationStatus 
dbInitializationMutex::initialize(char const* name)
{
    struct sembuf sops[4];
    char* path = (char*)name;
    if (strchr(name, '/') == NULL) { 
	path = new char[strlen(name)+strlen(keyFileDir)+1];
	sprintf(path, "%s%s", keyFileDir, name);
    }
    int fd = open(path, O_WRONLY|O_CREAT, 0777);
    if (fd < 0) {
	PRINT_ERROR("open");
   OsSysLog::add(FAC_DB, PRI_ERR, "Error attempting to open '%s' for writing.\n", path);
	if (path != name) { 
	    delete[] path;
	}
	return InitializationError;
    }
    ::close(fd);
    int key = ftok(path, '0');
    OsSysLog::add(FAC_DB, PRI_DEBUG,
                  "dbInitializationMutex::initialize path = '%s', key = 0x%x",
                  path, key);
    if (key < 0) {
	PRINT_ERROR("ftok");
        fprintf(stderr, "Error calling ftok('%s').\n", path);
	if (path != name) { 
	delete[] path;
    }
	return InitializationError;
    }
    if (path != name) { 
	delete[] path;
    }
    while (true) { 
       OsSysLog::add(FAC_DB, PRI_DEBUG,
                     "dbInitializationMutex::initialize semget(0x%x, 3, IPC_CREAT|0777)",
                     key);
	semid = semget(key, 3, IPC_CREAT|0777);
	if (semid < 0) { 
	    PRINT_ERROR("semget");
            OsSysLog::add(FAC_DB, PRI_CRIT,
                          "sem_init semget(3) failed: key = 0x%x",
                          key);
	    return InitializationError;
	}
	// Semaphore 0 - number of active processes
	// Semaphore 1 - intialization in progress (1 while initialization, 0 after it)
	// Semaphore 2 - semaphore was destroyed
	
	sops[0].sem_num = 0;
	sops[0].sem_op  = 0; /* check if semaphore was already initialized */
	sops[0].sem_flg = IPC_NOWAIT;
	sops[1].sem_num = 0;
	sops[1].sem_op  = 1; /* increment number of active processes */
	sops[1].sem_flg = SEM_UNDO;
	sops[2].sem_num = 1;
	sops[2].sem_op  = 1; /* initialization in process */
	sops[2].sem_flg = SEM_UNDO;
	sops[3].sem_num = 2;
	sops[3].sem_op  = 0; /* check if semaphore was destroyed */
	sops[3].sem_flg = IPC_NOWAIT;
	if (semop(semid, sops, 4) < 0) { 
	    if (errno == EAGAIN) { 
		sops[0].sem_num = 0;
		sops[0].sem_op  = -1; /* check if semaphore was already initialized */
		sops[0].sem_flg = SEM_UNDO|IPC_NOWAIT;
		sops[1].sem_num = 1;
		sops[1].sem_op  = 0; /* wait until inialization completed */
		sops[1].sem_flg = 0;
		sops[2].sem_num = 0;
		sops[2].sem_op  = 2; /* increment number of active processes */
		sops[3].sem_flg = SEM_UNDO;
		sops[3].sem_num = 2;
		sops[3].sem_op  = 0; /* check if semaphore was destroyed */
		sops[3].sem_flg = IPC_NOWAIT;
		if (semop(semid, sops, 4) == 0) { 
		    return AlreadyInitialized;
		}
		if (errno == EAGAIN) { 
		    sleep(1);
		    continue;
		}
	    } 
	    if (errno == EIDRM) {
		continue;
	    }
	    PRINT_ERROR("semop");
	    return InitializationError;
	} else { 
	    return NotYetInitialized;
	}
    }
}

void dbInitializationMutex::done() 
{
    struct sembuf sops[1];
    sops[0].sem_num = 1;
    sops[0].sem_op  = -1; /* initialization done */
    sops[0].sem_flg = SEM_UNDO;
    int rc = semop(semid, sops, 1);
    assert(rc == 0);
} 

bool dbInitializationMutex::close()
{
    int rc;
    struct sembuf sops[3];
    while (true) { 
	sops[0].sem_num = 0;
	sops[0].sem_op  = -1; /* decrement process couter */
	sops[0].sem_flg = SEM_UNDO;
	sops[1].sem_num = 0;
	sops[1].sem_op  = 0;  /* check if there are no more active processes */
	sops[1].sem_flg = IPC_NOWAIT;
	sops[2].sem_num = 2;
	sops[2].sem_op  = 1;  /* mark as destructed */
	sops[2].sem_flg = SEM_UNDO;
	if ((rc = semop(semid, sops, 3)) == 0) { 
	    return true;
	} else { 
	    assert(errno == EAGAIN);
	}
	sops[0].sem_num = 0;
	sops[0].sem_op  = -2; /* decrement process couter and check for non-zero */
	sops[0].sem_flg = SEM_UNDO|IPC_NOWAIT;
	sops[1].sem_num = 0;
	sops[1].sem_op  = 1;  
	sops[1].sem_flg = SEM_UNDO;
	if ((rc = semop(semid, sops, 2)) == 0) { 
	    return false;
	} else { 
	    assert(errno == EAGAIN);
	}
    }
}

void dbInitializationMutex::erase()
{
    semctl(semid, 0, IPC_RMID, &u);
}

#endif // USE_POSIX_API

#ifndef NO_PTHREADS

#if defined(_SC_NPROCESSORS_ONLN) 
int dbThread::numberOfProcessors() { 
    return sysconf(_SC_NPROCESSORS_ONLN); 
}
#elif defined(__linux__)
#include <linux/smp.h>
int dbThread::numberOfProcessors() { return smp_num_cpus; }
#elif defined(__FreeBSD__)
#include <sys/sysctl.h>
int dbThread::numberOfProcessors() { 
    int mib[2],ncpus=0;
    size_t len=sizeof(ncpus);
    mib[0]= CTL_HW;
    mib[1]= HW_NCPU;
    sysctl(mib,2,&ncpus,&len,NULL,0);
    return ncpus; 
}
#else
int dbThread::numberOfProcessors() { return 1; }
#endif
#endif // NO_PTHREADS


#else // _WIN32

unsigned dbSystem::getCurrentTimeMsec()
{
    return GetTickCount();
}

#ifdef SET_NULL_DACL
dbNullSecurityDesciptor dbNullSecurityDesciptor::instance;
#endif

#endif

void thread_proc dbPooledThread::pooledThreadFunc(void* arg)
{
    ((dbPooledThread*)arg)->run();
}

dbPooledThread::dbPooledThread(dbThreadPool* threadPool)
{
    pool = threadPool;
    startSem.open();
    readySem.open();
    next = NULL;
    running = true;
    thread.create(&pooledThreadFunc, this);
}

dbPooledThread::~dbPooledThread()
{
    startSem.close();
    readySem.close();
}

void dbPooledThread::stop() 
{
    running = false;
    startSem.signal(); 
    readySem.wait(pool->mutex);
}

void dbPooledThread::run() 
{
    dbCriticalSection cs(pool->mutex);
    while (true) { 
        startSem.wait(pool->mutex);
        if (!running) { 
            break;
        }
        (*f)(arg);
        readySem.signal();
    }
    readySem.signal();
}
    
void dbThreadPool::join(dbPooledThread* thr) 
{ 
    dbCriticalSection cs(mutex);
    thr->readySem.wait(mutex);
    thr->next = freeThreads;
    freeThreads = thr;
}


dbPooledThread* dbThreadPool::create(dbThread::thread_proc_t f, void* arg)
{
    dbCriticalSection cs(mutex);
    dbPooledThread* t = freeThreads;
    if (t == NULL) { 
        t = freeThreads = new dbPooledThread(this);
    }
    freeThreads = t->next;
    t->f = f;
    t->arg = arg;
    t->startSem.signal();
    return t;
}


dbThreadPool::dbThreadPool()
{
    freeThreads = NULL;
}
   
dbThreadPool::~dbThreadPool()
{
    dbCriticalSection cs(mutex);
    dbPooledThread *t, *next;
    for (t = freeThreads; t != NULL; t = next) { 
        next = t->next;
        t->stop();
        delete t;
    }        
}


