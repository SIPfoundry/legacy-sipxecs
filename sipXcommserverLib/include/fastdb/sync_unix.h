//-< SYNC_UNIX.H >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Intertask synchonization primitives for Unix platforms
//-------------------------------------------------------------------*--------*

#ifndef __SYNC_UNIX_H__
#define __SYNC_UNIX_H__

// Standard includes for all Unix platforms
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <assert.h>
#include <errno.h>

extern char const* keyFileDir; // default value: "/tmp/" 

#if !defined(USE_POSIX_SEMAPHORES) || !defined(USE_POSIX_MMAP) || !USE_POSIX_MMAP
#include <sys/ipc.h> 
#endif

#if defined(USE_POSIX_SEMAPHORES)
#include <sys/ipc.h> 
#include <semaphore.h>  // For POSIX style semaphores
#else
#include <sys/sem.h>    // For SysV style semaphores
#endif

#if defined(USE_POSIX_MMAP) && USE_POSIX_MMAP
#include <sys/mman.h>   // For mmap()
#else
#include <sys/shm.h>    
#include <sys/mman.h>
#endif

BEGIN_FASTDB_NAMESPACE

#define thread_proc

//////////////////////////////////////////////////////////
// If this system uses pthread based threads, then define
//   dbMutex(), dbThread(), dbLocalEvent(), etc as pthread-based implemenations

#ifndef NO_PTHREADS

// Use pthread based implementation
#include <pthread.h>

class dbMutex { 
    friend class dbLocalEvent;
    friend class dbLocalSemaphore;
    pthread_mutex_t cs;
    bool            initialized;
  public:
    dbMutex() {
        int rc = pthread_mutex_init(&cs, NULL);
        assert(rc == 0);
        initialized = true;
    }
    ~dbMutex() {
        int rc = pthread_mutex_destroy(&cs);
        assert(rc == 0);
        initialized = false;
    }
    bool isInitialized() { 
        return initialized;
    }
    void lock() {
        if (initialized) { 
            int rc = pthread_mutex_lock(&cs);
            assert(rc == 0);
        }
    }
    void unlock() {
        if (initialized) { 
            int rc = pthread_mutex_unlock(&cs);
            assert(rc == 0);
        }
    }
};


const size_t dbThreadStackSize = 1024*1024;

class dbThread { 
    pthread_t thread;
  public:
    typedef void (thread_proc* thread_proc_t)(void*);
    
    static void sleep(time_t sec) { 
        ::sleep(sec);
    }

    void create(thread_proc_t f, void* arg) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
#if !defined(__linux__)
        pthread_attr_setstacksize(&attr, dbThreadStackSize);
#endif
#if defined(_AIX41)
        // At AIX 4.1, 4.2 threads are by default created detached
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_UNDETACHED);
#endif
        pthread_create(&thread, &attr, (void*(*)(void*))f, arg);
        pthread_attr_destroy(&attr);
    }

    void join() { 
        void* result;
        pthread_join(thread, &result);
    }
    void detach() { 
        pthread_detach(thread);
    }

    enum ThreadPriority { 
        THR_PRI_LOW, 
        THR_PRI_HIGH
    };
    void setPriority(ThreadPriority pri) { 
#if defined(PRI_OTHER_MIN) && defined(PRI_OTHER_MAX)
        struct sched_param sp;
        sp.sched_priority = pri == THR_PRI_LOW ? PRI_OTHER_MIN : PRI_OTHER_MAX;
        pthread_setschedparam(thread, SCHED_OTHER, &sp); 
#endif
    }

    static int numberOfProcessors();
};


class dbLocalEvent { 
    pthread_cond_t   cond;
    int              signaled;
  public:
    void wait(dbMutex& mutex) { 
        while (!signaled) { 
            pthread_cond_wait(&cond, &mutex.cs);
        }
    }
    bool wait(dbMutex& mutex, time_t timeout) {
        if (!signaled) { 
            struct timespec abs_ts; 
#ifdef PTHREAD_GET_EXPIRATION_NP
            struct timespec rel_ts; 
            rel_ts.tv_sec = timeout/1000; 
            rel_ts.tv_nsec = timeout%1000*1000000;
            pthread_get_expiration_np(&rel_ts, &abs_ts);
#else
            struct timeval cur_tv;
            gettimeofday(&cur_tv, NULL);
            abs_ts.tv_sec = cur_tv.tv_sec + timeout/1000; 
            abs_ts.tv_nsec = cur_tv.tv_usec*1000 + timeout%1000*1000000;
            if (abs_ts.tv_nsec > 1000000000) { 
                abs_ts.tv_nsec -= 1000000000;
                abs_ts.tv_sec += 1;
            }
#endif
            do { 
                int rc = pthread_cond_timedwait(&cond, &mutex.cs, &abs_ts);
                if (rc != 0) {
                    return false;
                }
            } while (!signaled);

        }
        return true;
    }
    void signal() {
        signaled = true;
        pthread_cond_broadcast(&cond);
    }
    void reset() {
        signaled = false;
    }
    void open(bool initValue = false) { 
        signaled = initValue;
        pthread_cond_init(&cond, NULL);
    }
    void close() {
        pthread_cond_destroy(&cond);
    }
};

class dbLocalSemaphore { 
    pthread_cond_t   cond;
    int              count;
  public:
    void wait(dbMutex& mutex) { 
        while (count == 0) { 
            pthread_cond_wait(&cond, &mutex.cs);
        }
        count -= 1;
    }
    bool wait(dbMutex& mutex, time_t timeout) {
        if (count == 0) { 
            struct timespec abs_ts; 
#ifdef PTHREAD_GET_EXPIRATION_NP
            struct timespec rel_ts; 
            rel_ts.tv_sec = timeout/1000; 
            rel_ts.tv_nsec = timeout%1000*1000000;
            pthread_get_expiration_np(&rel_ts, &abs_ts);
#else
            struct timeval cur_tv;
            gettimeofday(&cur_tv, NULL);
            abs_ts.tv_sec = cur_tv.tv_sec + timeout/1000; 
            abs_ts.tv_nsec = cur_tv.tv_usec*1000 + timeout%1000*1000000;
            if (abs_ts.tv_nsec > 1000000000) { 
                abs_ts.tv_nsec -= 1000000000;
                abs_ts.tv_sec += 1;
            }
#endif
            do { 
                int rc = pthread_cond_timedwait(&cond, &mutex.cs, &abs_ts);
                if (rc != 0) { 
                    return false;
                }
            } while (count == 0);
        }
        count -= 1;
        return true;
    }
    void signal(unsigned inc = 1) {
        count += inc;
        if (inc > 1) { 
            pthread_cond_broadcast(&cond);
        } else if (inc == 1) { 
            pthread_cond_signal(&cond);
        }
    }
    void open(unsigned initValue = 0) { 
        pthread_cond_init(&cond, NULL);
        count = initValue;
    }
    void close() {
        pthread_cond_destroy(&cond);
    }
};

template<class T> 
class dbThreadContext { 
    pthread_key_t key;
  public:
    T* get() { 
        return (T*)pthread_getspecific(key);
    }
    void set(T* value) { 
        pthread_setspecific(key, value);
    }
    dbThreadContext() { 
        pthread_key_create(&key, NULL);
    }
    ~dbThreadContext() { 
        pthread_key_delete(key);
    }
};

class dbProcessId { 
    pid_t     pid;
    pthread_t tid;
  public:
    bool operator != (dbProcessId const& other) const { 
        return pid != other.pid || tid != other.tid;
    }

    void clear() { 
        pid = 0;
        tid = 0;
    }

    static dbProcessId getCurrent() {
        dbProcessId curr;
        curr.pid = getpid();
        curr.tid = pthread_self();
        return curr;
    }
};

#else // NO_PTHREAD

// Non pthread based threads, mutexes, etc.
// Maps to skeleton  functions, this implementation isn't using threads.

class dbMutex {
    bool initialized;

   public:
    dbMutex() {
        initialized = true;
    }

    ~dbMutex() { 
        initialized = false;
    }

    bool isInitialized() { 
        return initialized;
    }

    void lock() {}
    void unlock() {}
};

class dbThread { 
  public:
    typedef void (thread_proc* thread_proc_t)(void*);
    void create(thread_proc_t f, void* arg) { f(arg); }
    void join() {}
    void detach() {}
    enum ThreadPriority { 
        THR_PRI_LOW, 
        THR_PRI_HIGH
    };
    void setPriority(ThreadPriority pri) { }
    static int numberOfProcessors() { return 1; }
};

class dbLocalSemaphore { 
    int count;
  public:
    void wait(dbMutex&) { 
        assert (count > 0);
        count -= 1;
    }
    void signal(unsigned inc = 1) {
        count += inc;
    }
    void open(unsigned initValue = 0) {
        count = initValue;
    }
    void close() {}
};

class dbLocalEvent { 
    bool signaled;
  public:
    void wait(dbMutex&) { 
        assert(signaled);
    }
    bool wait(dbMutex& mutex, time_t timeout) {
        return true;
    }
    void signal() {
        signaled = true;
    }
    void reset() {
        signaled = false;
    }
    void open(bool initValue = false) {
        signaled = initValue;
    }
    void close() {}
};

template<class T>
class dbThreadContext { 
    T* value;
  public:
    T* get() { 
        return value;
    }
    void set(T* value) { 
        this->value = value;
    }
    dbThreadContext() { value = NULL; }
};


class dbProcessId { 
    pid_t       pid;
  public:
    bool operator != (dbProcessId const& other) const { 
        return pid != other.pid;
    }
    
    void clear() { 
        pid = 0;
    }

    static dbProcessId getCurrent() {
        dbProcessId curr;
        curr.pid = getpid();
        return curr;
    }
};

#endif // NO_PTHREAD


#define INFINITE (~0U)


#ifdef USE_POSIX_SEMAPHORES

// Initialization Mutex using Posix based semaphores
class dbInitializationMutex { 
    sem_t* sem;
  public: 
    enum initializationStatus { 
        InitializationError, 
        AlreadyInitialized,
        NotYetInitialized
    };
    initializationStatus initialize(char const* name) { 
        initializationStatus status;
        char *tmp = new char[strlen(name) + 32 + 1];
        if (name[0] != '/')
        {
           // Want to have the full path, but sem_open doesn't allow
           // slashes in the name.  Use ftok of the path to get close 
           // enough.
           sprintf(tmp, "%0xd-%s", ftok(keyFileDir, 0), name);
        }
        else
        {
           strcpy(tmp, name);
        }
        while (true) {
            sem = sem_open(tmp, 0);
            if (sem == SEM_FAILED) { 
                if (errno == ENOENT) {
                    sem = sem_open(tmp, O_CREAT|O_EXCL, 0777, 0);
                    if (sem != SEM_FAILED) { 
                        status = NotYetInitialized;
                        break;
                    } else if (errno != EEXIST) { 
                        status = InitializationError;
                        break;
                    }
                } else { 
                    status = InitializationError;
                    break;
                }
            } else { 
                status = (sem_wait(sem) == 0) 
                    ? AlreadyInitialized : InitializationError;
                break;
            }
        }
        delete[] tmp;
        return status;
    }

    void done() { 
        sem_post(sem);
    }
    bool close() {
        sem_close(sem);
        return false;
    }
    void erase() { 
        close();
    }
};

class dbSemaphore { 
  protected:
    sem_t* sem;
    char *semName ;

  public:
    void wait() { 
        for(;;)
        {
            int rc = sem_wait(sem);
            if (rc != 0 && errno == EINTR)
               continue;
            assert(rc == 0);
            break;
        }
    }

    bool wait(unsigned msec) { 
#ifdef POSIX_1003_1d
        struct timespec abs_ts;
        struct timeval  cur_tv;
        clock_gettime(CLOCK_REALTIME, &cur_tv);
        abs_ts.tv_sec = cur_tv.tv_sec + (msec + cur_tv.tv_usec / 1000) / 1000000; 
        abs_ts.tv_nsec = (msec + cur_tv.tv_usec / 1000) % 1000000 * 1000;
        for(;;)
        {
            int rc = sem_timedwait(sem, &abs_ts);
            if (rc < 0) { 
                if (errno == EINTR)
                   continue ;
                assert(errno == ETIMEDOUT);
                return false;
            }
        }
        return true;
#else 
        for(;;)
        {
            int rc = sem_wait(sem);
            if (rc != 0 && errno == EINTR)
               continue;
            assert(rc == 0);
            return true;
        }
#endif  
    }

    void signal(unsigned inc = 1) {
        // For every non-zero inc, post to the sem once
        while (inc-- != 0) { 
            sem_post(sem);
        }
    }
    void reset() { 
        while (sem_trywait(sem) == 0);
    }    
    bool open(char const* name, unsigned initValue = 0) {
        char *tmp = new char[strlen(name) + 32 + 1];
        if (name[0] != '/')
        {
           // Want to have the full path, but sem_open doesn't allow
           // slashes in the name.  Use ftok of the path to get close 
           // enough.
           sprintf(tmp, "%0xd-%s", ftok(keyFileDir, 0), name);
        }
        else
        {
           strcpy(tmp, name);
        }
        semName = strdup(tmp) ;
        sem = sem_open(tmp, O_CREAT, 0777, initValue);
        delete[] tmp;
        return sem != NULL; 
    }
    void close() {
        sem_close(sem);
    }
    void erase() { 
        close();
        free(semName) ;
    }
};

class dbEvent : public dbSemaphore { 
  public:
    void wait() { 
        dbSemaphore::wait();
        sem_post(sem);
    }
    bool wait(unsigned msec) { 
        if (dbSemaphore::wait(msec)) { 
            sem_post(sem);
            return true;
        }
        return false;
    }
    void signal() {
        while (sem_trywait(sem) == 0);
        sem_post(sem);
    }
    void reset() {
        while (sem_trywait(sem) == 0);
    }
    bool open(char const* name, bool signaled = false) {
        bool okay =  dbSemaphore::open(name, (int)signaled);
        return okay ;
    }
};
#else // USE_POSIX_SEMAPHORES

class FASTDB_DLL_ENTRY dbWatchDog { 
    bool open(char const* name, int flags);
  public:
    bool watch();
    void close(); 
    bool open(char const* name);
    bool create(char const* name);
    int id;
};

// Define local implemenation of InitializationMutex in sync.cpp
class dbInitializationMutex { 
    int semid;
  public: 
    enum initializationStatus { 
        InitializationError, 
        AlreadyInitialized,
        NotYetInitialized
    };
    initializationStatus initialize(char const* name);
    void done(); 
    bool close();
    void erase();
};


class dbSemaphore { 
    int s;
  public:
    bool wait(unsigned msec = INFINITE);
    void signal(unsigned inc = 1);
    bool open(char const* name, unsigned initValue = 0);
    void reset();
    void close();
    void erase();
};

class dbEvent { 
    int e;
  public:
    bool wait(unsigned msec = INFINITE);
    void signal();
    void reset();
    bool open(char const* name, bool signaled = false);
    void close();
    void erase();
};
#endif // USE_POSIX_SEMAPHORES


// Define dbSharedObject and dbSharedMemory
#if defined(USE_POSIX_MMAP) && USE_POSIX_MMAP

// For POSIX dbSharedObject, we use mmap()
template<class T>
class dbSharedObject { 
    char* name;
    T*  ptr;
    int fd;
  public:

    dbSharedObject() { 
        name = NULL;
        ptr = NULL;
        fd = -1;
    }

    int open(char* fileName) { 
        bool created = false ;
        delete[] name;
        name = new char[strlen(fileName) + strlen(keyFileDir) + 1];
        if (fileName[0] != '/')
        {
           sprintf(name, "%s%s", keyFileDir, fileName);
        }
        else
        {
           strcpy(name, fileName);
        }
        // See if it already exists ;
        fd = ::open(name, O_RDWR);
        if (fd < 0) { 
            // Nope, see if we can create it
            fd = ::open(name, O_RDWR | O_CREAT, 0777);
            if (fd < 0) 
               return -1;

            created = true ;
            if (ftruncate(fd, sizeof(T)) < 0) {
                ::close(fd);
                return -1;
            }
        }
        ptr = (T*)mmap(NULL,
                       DOALIGN(sizeof(T), 4096),
                       PROT_READ|PROT_WRITE,
                       MAP_SHARED,
                       fd,
                       0);
        if (ptr == MAP_FAILED) { 
            ptr = NULL;
            ::close(fd);
            return -1;
        }
        return created ? 1 : 0 ;
    }

    T* get() { return ptr; }

    void close() { 
        if (ptr != NULL) { 
            munmap((char*)ptr, DOALIGN(sizeof(T), 4096));
        }
        if (fd > 0) { 
            ::close(fd);
            fd = -1 ;
        }
    }
    void erase() {
        close();
        unlink(name);   
    }  

    int getFd() {
       return fd ;
    }

    ~dbSharedObject() { 
        delete[] name;
    }
};

#else // USE_POSIX_MMAP

// Non POSIX, internal implementations of SharedMemory and SharedObject
class dbSharedMemory { 
  protected:
    char*  ptr;
    int    shm;

  public:
    bool  open(char const* name, size_t size); 
    void  close();
    void  erase(); 
    char* get_base() { 
        return ptr;
    }
};

template<class T>
class dbSharedObject : public dbSharedMemory { 
  public:
    int open(char* name) { 
        return dbSharedMemory::open(name, sizeof(T)) ? 0 : -1 ;
    }
    T* get() { return (T*)ptr; }
};

#endif

//////////////////////////////////////////////////////////////////////////
// Define dBGlobalCriticalSection for various platforms

// QNX uses a pthread based mutex for its implementation
//     Use only if pthread support is also enabled, else we'll use the default case
#if defined(__QNX__) && !defined(NO_PTHREADS)
typedef pthread_mutex_t sharedsem_t;

class dbGlobalCriticalSection { 
    pthread_mutexattr_t attr;
    sharedsem_t* sem;
  public:
    void enter() {
        int rc = pthread_mutex_lock(sem);
        assert(rc == 0);
    }
    void leave() { 
        int rc = pthread_mutex_unlock(sem);
        assert(rc == 0);
    }
    bool open(char const*, sharedsem_t* shr) { 
        sem = shr;
        return true;
    }
    bool create(char const*, sharedsem_t* shr) { 
        sem = shr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutexattr_setrecursive(&attr, PTHREAD_RECURSIVE_ENABLE);
        pthread_mutex_init(sem, &attr);
        return true;
    }
    void close() {}
    void erase() {
        pthread_mutex_destroy(sem);
    }
};


#elif defined(__osf__) && !defined(RECOVERABLE_CRITICAL_SECTION)
// OSF uses "shared memory semaphores", located within a region mapped with mmap().
#include <errno.h>
typedef msemaphore sharedsem_t;

class dbGlobalCriticalSection { 
    sharedsem_t* sem;
  public:
    void enter() { 
        int rc;
        while ((rc = msem_lock(sem, 0)) < 0 && errno == EINTR);
        assert(rc == 0);
    }
    void leave() { 
        int rc = msem_unlock(sem, 0);
        assert(rc == 0);        
    }
    bool open(char const*, sharedsem_t* shr) { 
        sem = shr;
        return true;
    }
    bool create(char const*, sharedsem_t* shr) { 
        sem = shr;
        msem_init(shr, MSEM_UNLOCKED);
        return true;
    }
    void close() {}
    void erase() {
        msem_remove(sem);
    }
};
        

#elif defined(__sun) && !defined(RECOVERABLE_CRITICAL_SECTION)
// Sun uses the Solaris style semaphore implemenation (sema_init(), sema_post())
#include <synch.h>
#include <errno.h>
typedef sema_t sharedsem_t;

class dbGlobalCriticalSection { 
    sharedsem_t* sem;
  public:
    void enter() { 
        int rc;
        while ((rc = sema_wait(sem)) < 0 && errno == EINTR);
        assert(rc == 0);
    }
    void leave() { 
        int rc = sema_post(sem);
        assert(rc == 0);
    }
    bool open(char const*, sharedsem_t* shr) { 
        sem = shr;
        return true;
    }
    bool create(char const*, sharedsem_t* shr) { 
        sem = shr;
        return sema_init(shr, 1, USYNC_PROCESS, NULL) == 0;
    }
    void close() {}
    void erase() {
        sema_destroy(sem);
    }
};

#elif defined(USE_POSIX_SEMAPHORES) && !defined(RECOVERABLE_CRITICAL_SECTION)
// Everyone else uses the POSIX style semaphores (sem_wait(), sem_post(), etc) if defined
typedef sem_t sharedsem_t;

class dbGlobalCriticalSection { 
    sharedsem_t* sem;
    char *semName ;

  public:
    dbGlobalCriticalSection() {
       semName = NULL ;
    }

    void enter() { 
        for(;;)
        {
            int rc = sem_wait(sem);
            if (rc != 0 && errno == EINTR)
               continue;
            assert(rc == 0);
            break;
        }
    }
    void leave() { 
        int rc = sem_post(sem);
        assert(rc == 0);
    }
    bool open(char const* name, sharedsem_t* shr) { 
        sem = shr;
        if (!semName)
        {
           semName = strdup(name) ;
        }
        return true;
    }

    bool create(char const* name, sharedsem_t* shr) {   
        sem = shr;
        semName = strdup(name) ;
        return sem_init(sem, 1, 1) == 0;
    }

    void close() {
        if (semName)
        {
            free(semName) ;
            semName = NULL ;
        }
    }
    void erase() { 
        sem_destroy(sem);
        if (semName)
        {
            free(semName) ;
            semName = NULL ;
        }
    }
};

#else

#define USE_LOCAL_CS_IMPL

#define GLOBAL_CS_DEBUG 1

// Lastly, use the local implementation
typedef long sharedsem_t;

class dbGlobalCriticalSection { 
    int          semid;
    sharedsem_t* count;
#if GLOBAL_CS_DEBUG
    pthread_t    owner;
#endif

  public:
    void enter(); 
    void leave();
    bool open(char const* name, sharedsem_t* shr);
    bool create(char const* name, sharedsem_t* shr);
    void close() {}
    void erase();
};
#endif //dbGLobalCriticalSection switch

END_FASTDB_NAMESPACE

#endif //__SYNC_UNIX_H__
