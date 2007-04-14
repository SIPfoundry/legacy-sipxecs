//-< SYNC.H >--------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Intertask synchonization primitives
//-------------------------------------------------------------------*--------*

#ifndef __SYNC_H__
#define __SYNC_H__

class FASTDB_DLL_ENTRY dbSystem { 
  public:
    static unsigned getCurrentTimeMsec();
};

#ifdef _WIN32

#ifdef SET_NULL_DACL
class FASTDB_DLL_ENTRY dbNullSecurityDesciptor { 
  public:
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa; 

    dbNullSecurityDesciptor() { 
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE; 
    sa.lpSecurityDescriptor = &sd;
    }
    
    static dbNullSecurityDesciptor instance;
};
#define FASTDB_SECURITY_ATTRIBUTES &dbNullSecurityDesciptor::instance.sa
#else    
#define FASTDB_SECURITY_ATTRIBUTES NULL
#endif

class FASTDB_DLL_ENTRY dbMutex { 
    CRITICAL_SECTION cs;
  public:
    dbMutex() { 
    InitializeCriticalSection(&cs);
    }
    ~dbMutex() { 
    DeleteCriticalSection(&cs);
    }
    void lock() { 
    EnterCriticalSection(&cs);
    }
    void unlock() { 
    LeaveCriticalSection(&cs);
    }
};

#define thread_proc WINAPI

class FASTDB_DLL_ENTRY dbThread { 
    HANDLE h;
  public:
    typedef void (thread_proc* thread_proc_t)(void*);

    void create(thread_proc_t f, void* arg) { 
    DWORD threadid;
	h = CreateThread(FASTDB_SECURITY_ATTRIBUTES, 0, LPTHREAD_START_ROUTINE(f), arg,
			 0, &threadid);
    }
    enum ThreadPriority { 
    THR_PRI_LOW, 
    THR_PRI_HIGH
    };

    void setPriority(ThreadPriority pri) { 
    SetThreadPriority(h, pri == THR_PRI_LOW ? THREAD_PRIORITY_IDLE : THREAD_PRIORITY_HIGHEST);
    }
    
    void join() { 
    WaitForSingleObject(h, INFINITE);
    CloseHandle(h);
    h = NULL;
    }
    void detach() { 
    if (h != NULL) { 
        CloseHandle(h);
        h = NULL;
    }
    }   
    dbThread() { 
    h = NULL; 
    }
    ~dbThread() { 
    if (h != NULL) { 
        CloseHandle(h);
    }
    }
    static int numberOfProcessors() { 
#ifdef PHAR_LAP
    return 1;
#else
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#endif
    }
};
    
class FASTDB_DLL_ENTRY dbProcessId { 
    DWORD tid;
  public:
    bool operator != (dbProcessId const& other) const { 
    return tid != other.tid;
    }

    void clear() { 
    tid = 0;
    }

    static dbProcessId getCurrent() {
    dbProcessId curr;
    curr.tid = GetCurrentThreadId();
    return curr;
    }

    // rschaaf: Temporary addition to aid debugging
    DWORD getTid() {
       return tid;
    }

    DWORD getPid() {
       return tid;
    }
};

class FASTDB_DLL_ENTRY dbInitializationMutex { 
    HANDLE m;
  public: 
    enum initializationStatus { 
    InitializationError, 
    AlreadyInitialized,
    NotYetInitialized
    };
    initializationStatus initialize(char const* name) { 
    initializationStatus status;
    m = CreateMutex(FASTDB_SECURITY_ATTRIBUTES, true, name);
    if (GetLastError() == ERROR_ALREADY_EXISTS) { 
        status = WaitForSingleObject(m, INFINITE) == WAIT_OBJECT_0 
           ? AlreadyInitialized : InitializationError;
        ReleaseMutex(m);
    } else if (m != NULL) { 
        status = NotYetInitialized;
    } else { 
        status = InitializationError;
    }
    if (status == InitializationError)
    {
       OsSysLog::add(FAC_DB, PRI_CRIT, "dbInitializationMutex::initialize returning InitializationError, name = '%s'",
                     name);
    }
    return status;
    }
    void done() { 
    ReleaseMutex(m);
    }
    bool close() {
    CloseHandle(m);
    return false;
    }
    void erase() { 
    close();
    }
    dbInitializationMutex() { 
        m = NULL;
    }
};


const int dbMaxSemValue = 1000000;


class FASTDB_DLL_ENTRY dbSemaphore { 
  protected:
    HANDLE s;
  public:
    bool wait(unsigned msec = INFINITE) { 
    int rc = WaitForSingleObject(s, msec);
    assert(rc == WAIT_OBJECT_0 || rc == WAIT_TIMEOUT);
    return rc == WAIT_OBJECT_0;
    }
    void signal(unsigned inc = 1) {
    if (inc != 0) { 
        ReleaseSemaphore(s, inc, NULL);
    }
    }
    void reset() { 
    while (WaitForSingleObject(s, 0) == WAIT_OBJECT_0);
    }    
    bool open(char const* name, unsigned initValue = 0) {
    s = CreateSemaphore(FASTDB_SECURITY_ATTRIBUTES, initValue, dbMaxSemValue, name);
    return s != NULL; 
    }
    void close() {
    CloseHandle(s);
    }
    void erase() { 
    close();
    }
    dbSemaphore() { 
        s = NULL;
    }
};

class FASTDB_DLL_ENTRY dbEvent { 
  protected:
    HANDLE e;
  public:
    bool wait(unsigned msec = INFINITE) { 
    int rc = WaitForSingleObject(e, msec);
    assert(rc == WAIT_OBJECT_0 || rc == WAIT_TIMEOUT);
    return rc == WAIT_OBJECT_0;
    }
    void signal() {
    SetEvent(e);
    }
    void reset() {
    ResetEvent(e);
    }
    bool open(char const* name, bool signaled = false) {
    e = CreateEvent(FASTDB_SECURITY_ATTRIBUTES, true, signaled, name);
    return e != NULL; 
    }
    void close() {
    CloseHandle(e);
    }
    void erase() { 
    close();
    }
    dbEvent() { 
        e = NULL;
    }
};

class FASTDB_DLL_ENTRY dbLocalSemaphore : public dbSemaphore { 
  public:
    bool wait(dbMutex& mutex, time_t timeoutMsec) { 
    mutex.unlock();
    int rc = WaitForSingleObject(s, timeoutMsec);
    assert(rc == WAIT_OBJECT_0 || rc == WAIT_TIMEOUT);
    mutex.lock();
    return rc == WAIT_OBJECT_0;
    }
    void wait(dbMutex& mutex) { 
    mutex.unlock();
    int rc = WaitForSingleObject(s, INFINITE);
    assert(rc == WAIT_OBJECT_0);
    mutex.lock();
    }
    bool open(unsigned initValue = 0) {
    return dbSemaphore::open(NULL, initValue);
    }
};

class FASTDB_DLL_ENTRY dbLocalEvent : public dbEvent { 
  public:
    bool wait(dbMutex& mutex, time_t timeoutMsec) { 
    mutex.unlock();
    int rc = WaitForSingleObject(e, timeoutMsec);
    assert(rc == WAIT_OBJECT_0 || rc == WAIT_TIMEOUT);
    mutex.lock();
    return rc == WAIT_OBJECT_0;
    }
    void wait(dbMutex& mutex) { 
    mutex.unlock();
    int rc = WaitForSingleObject(e, INFINITE);
    assert(rc == WAIT_OBJECT_0);
    mutex.lock();
    }
    bool open(bool signaled = false) {
     return dbEvent::open(NULL, signaled);
     }
};

template<class T>
class dbThreadContext { 
    int index;
  public:
    T* get() { 
    return (T*)TlsGetValue(index);
    }
    void set(T* value) { 
    TlsSetValue(index, value);
    }
    dbThreadContext() { 
    index = TlsAlloc();
    assert(index != TLS_OUT_OF_INDEXES);
    }
    ~dbThreadContext() { 
    TlsFree(index);
    }
};

template<class T>
class dbSharedObject { 
    T*     ptr;
    HANDLE h;
  public:

    bool open(char* name) { 
#ifdef NO_MMAP
    ptr = new T();
#else
    h = CreateFileMapping(INVALID_HANDLE_VALUE,
                  FASTDB_SECURITY_ATTRIBUTES, PAGE_READWRITE, 0, 
                  sizeof(T), name);
    if (h == NULL) { 
        return false;
    }
    ptr = (T*)MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (ptr == NULL) { 
        CloseHandle(h);
        return false;
    }
#endif
    return true;
    }

    T* get() { return ptr; }

    void close() { 
#ifdef NO_MMAP
    delete[] ptr;
#else
    UnmapViewOfFile(ptr);
    CloseHandle(h);
#endif
    }
    void erase() { 
    close();
    }
    dbSharedObject() { 
        ptr = NULL;
        h = NULL;
    }
};

typedef long sharedsem_t;

class FASTDB_DLL_ENTRY dbGlobalCriticalSection { 
    HANDLE       event;
    sharedsem_t* count;

  public:
    void enter() { 
    if (InterlockedDecrement(count) != 0) { 
        // another process is in critical section
        int rc = WaitForSingleObject(event, INFINITE);
        assert (rc == WAIT_OBJECT_0);
    }
    }

    void leave() { 
    if (InterlockedIncrement(count) <= 0) { 
        // some other processes try to enter critical section
        SetEvent(event);
    }
    }

    bool open(char const* name, long* count) { 
    this->count = count;
    event = OpenEvent(EVENT_ALL_ACCESS, FALSE, name);
    return event != NULL;
    }
    bool create(char const* name, long* count) { 
    this->count = count;
    *count = 1;
    event = CreateEvent(FASTDB_SECURITY_ATTRIBUTES, false, false, name);
    return event != NULL;
    }
    void close() { 
    CloseHandle(event);
    }
    void erase() { 
    close();
    }
    dbGlobalCriticalSection() {
        event = NULL;
    }
        
};
    
    
#else // Unix

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#ifdef USE_POSIX_API
#include <semaphore.h>
#include <sys/mman.h>
#else
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/mman.h>
#endif

#define thread_proc

#ifndef NO_PTHREADS

#include <pthread.h>

class dbMutex { 
    friend class dbLocalEvent;
    friend class dbLocalSemaphore;
    pthread_mutex_t cs;
  public:
    dbMutex() { 
    pthread_mutex_init(&cs, NULL);
    }
    ~dbMutex() { 
    pthread_mutex_destroy(&cs);
    }
    void lock() { 
    pthread_mutex_lock(&cs);
    }
    void unlock() { 
    pthread_mutex_unlock(&cs);
    }
};

const size_t dbThreadStackSize = 1024*1024;

class dbThread { 
    pthread_t thread;
  public:
    typedef void (thread_proc* thread_proc_t)(void*);
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
    while (!signaled) {
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
        int rc = pthread_cond_timedwait(&cond, &mutex.cs, &abs_ts);
        if (rc == ETIMEDOUT) { 
        return false;
        }
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
    void wait(dbMutex& mutex, time_t timeout) {
    while (count == 0) {
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
        pthread_cond_timedwait(&cond, &mutex.cs, &abs_ts);
    }
    count -= 1;
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
    int       pid;
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

    // for Linux: Temporary addition to aid debugging
    int getTid() {
       return tid;
    }

    int getPid() {
       return pid;
    }
};

#else

class dbMutex { 
   public:
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
    int       pid;
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

#endif

#define INFINITE (~0U)

#ifdef USE_POSIX_API

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
       char* tmp = NULL;
       if (*name != '/') { 
          tmp = new char[strlen(name)+2];
          strcpy(tmp+1, name);
          *tmp = '/';
          name = tmp;
       }
       while (true) {
          sem = sem_open(name, 0);
          if (sem == NULL) { 
             if (errno == ENOENT) {
                sem = sem_open(name, O_CREAT|O_EXCL, 0777, 0);
                if (sem != NULL) { 
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
             status = (sem_wait(sem) == 0 && sem_post(sem) == 0) 
                ? AlreadyInitialized : InitializationError;
             break;
          }
       }
       if (status == InitializationError)
       {
          OsSysLog::add(FAC_DB, PRI_CRIT, "dbInitializationMutex::initialize returning InitializationError, name = '%s'",
                        name);
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
  public:
    void wait() { 
    int rc = sem_wait(sem);
    assert(rc == 0);
    }

    bool wait(unsigned msec) { 
#ifdef POSIX_1003_1d
    struct timespec abs_ts;
    struct timeval  cur_tv;
    clock_gettime(CLOCK_REALTIME, &cur_tv);
    abs_ts.tv_sec = cur_tv.tv_sec + (msec + tv.tv_usec / 1000) / 1000000; 
    abs_ts.tv_nsec = (msec + tv.tv_usec / 1000) % 1000000 * 1000;
    int rc = sem_timedwait(sem, &abs_ts);
    if (rc < 0) { 
        assert(errno == ETIMEDOUT);
        return false;
    }
    return true;
#else 
    int rc = sem_wait(sem);
    assert(rc == 0);
    return true;
#endif  
    }

    void signal(unsigned inc = 1) {
    while (--inc > 0) { 
        sem_post(sem);
    }
    }
    void reset() { 
    while (sem_trywait(sem) == 0);
    }    
    bool open(char const* name, unsigned initValue = 0) {
    char* tmp = NULL;
    if (*name != '/') { 
        tmp = new char[strlen(name)+2];
        strcpy(tmp+1, name);
        *tmp = '/';
        name = tmp;
    }
    sem = sem_open(name, O_CREAT, 0777, initValue);
    delete[] tmp;
    return sem != NULL; 
    }
    void close() {
    sem_close(sem);
    }
    void erase() { 
    close();
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
    return dbSemaphore::open(name, (int)signaled);
    }
};

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

    bool open(char* fileName) { 
    delete[] name;
    name = new char[strlen(fileName) + 1];
    strcpy(name, fileName);
    fd = ::open(fileName, O_RDWR|O_CREAT, 0777);
    if (fd < 0) { 
        return false;
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
        return false;
    }
    return true;
    }

    T* get() { return ptr; }

    void close() { 
    if (ptr != NULL) { 
        munmap(ptr, DOALIGN(sizeof(T), 4096));
    }
    if (fd > 0) { 
        ::close(fd);
    }
    }
    void erase() {
    close();
    unlink(name);   
    }  

    ~dbSharedObject() { 
    delete[] name;
    }
};

#else // USE_POSIX_API

extern char const* keyFileDir; // default value: "/tmp/" 

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
    bool open(char* name) { 
    return dbSharedMemory::open(name, sizeof(T));
    }
    T* get() { return (T*)ptr; }
};

#endif

#if defined(__QNX__)

#define LOG_SEM(sem) (int)((sem).__m_count)

typedef pthread_mutext_t sharedsem_t;

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


#elif defined(__osf__)

#define LOG_SEM(sem) (int)((sem).msem_state)

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
    

#elif defined(__sun)

#define LOG_SEM(sem) (int)((sem).count)

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

#elif defined(USE_POSIX_API)

#define LOG_SEM(sem) (*(int *)&(sem))

typedef sem_t sharedsem_t;

class dbGlobalCriticalSection { 
    sharedsem_t* sem;

  public:
    void enter() { 
    int rc = sem_wait(sem);
    assert(rc == 0);
    }
    void leave() { 
    int rc = sem_post(sem);
    assert(rc == 0);
    }
    bool open(char const* name, sharedsem_t* shr) { 
    sem = shr;
    return true;
    }
    bool create(char const* name, sharedsem_t* shr) {   
    sem = shr;
    return sem_init(sem, 1, 1) == 0;
    }
    void close() {}
    void erase() { 
    sem_destroy(sem);
    }
};

#else

#define LOG_SEM(sem) (int)(sem)

typedef long sharedsem_t;

class dbGlobalCriticalSection { 
    int          semid;
    sharedsem_t* count;

  public:
    void enter(); 
    void leave();
    bool open(char const* name, sharedsem_t* shr);
    bool create(char const* name, sharedsem_t* shr);
    void close() {}
    void erase();
};
#endif

#endif

class FASTDB_DLL_ENTRY dbCriticalSection { 
  private:
    dbMutex& mutex;
  public:
    dbCriticalSection(dbMutex& guard) : mutex(guard) {
    mutex.lock();
    }
    ~dbCriticalSection() { 
    mutex.unlock();
    }
};
    
#define SMALL_BUF_SIZE 512

class FASTDB_DLL_ENTRY dbSmallBuffer { 
  protected:
    char* buf;
    char  smallBuf[SMALL_BUF_SIZE];
    size_t used;

  public:
    dbSmallBuffer(size_t size) { 
    if (size > SMALL_BUF_SIZE) { 
        buf = new char[size];
    } else { 
        buf = smallBuf;
    }
        used = size;
    }

    dbSmallBuffer() { 
        used = 0;
	buf = smallBuf;
    }

    void put(size_t size) { 
        if (size > SMALL_BUF_SIZE && size > used) { 
            if (buf != smallBuf) { 
                delete[] buf;
            }
            buf = new char[size];
            used = size;
        }
    }

    operator char*() { return buf; }
    char* base() { return buf; }

    ~dbSmallBuffer() { 
    if (buf != smallBuf) { 
        delete[] buf;
    }
    }
};

class dbThreadPool;

class FASTDB_DLL_ENTRY dbPooledThread { 
  private:
    friend class dbThreadPool;

    dbThread                thread;
    dbThreadPool*           pool;
    dbPooledThread*         next;
    dbThread::thread_proc_t f;
    void*                   arg;
    bool                    running;
    dbLocalSemaphore        startSem;
    dbLocalSemaphore        readySem;
    
    static void thread_proc  pooledThreadFunc(void* arg);

    void run();
    void stop();

    dbPooledThread(dbThreadPool* threadPool); 
    ~dbPooledThread(); 
};

class FASTDB_DLL_ENTRY dbThreadPool { 
    friend class dbPooledThread;
    dbPooledThread* freeThreads;
    dbMutex         mutex;

  public:
    dbPooledThread* create(dbThread::thread_proc_t f, void* arg);
    void join(dbPooledThread* thr);
    dbThreadPool();
    ~dbThreadPool();
};    
    
#endif


