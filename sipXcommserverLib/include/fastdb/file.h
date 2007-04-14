//-< FILE.CPP >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// System independent intrface to mapped on memory file
//-------------------------------------------------------------------*--------*

#ifndef __FILE_H__
#define __FILE_H__

#include "sync.h"

#if defined(REPLICATION_SUPPORT)
const int dbModMapBlockBits = 12; // 10;
const int dbModMapBlockSize = 1 << dbModMapBlockBits;
#elif defined(NO_MMAP)
const int dbModMapBlockBits = 12;
const int dbModMapBlockSize = 1 << dbModMapBlockBits;
#endif

#ifdef REPLICATION_SUPPORT

class dbFile;
class dbReplicatedDatabase;

struct ReplicationRequest { 
    enum {
    RR_CONNECT, 
    RR_RECOVERY, 
    RR_GET_STATUS, 
    RR_STATUS, 
    RR_UPDATE_PAGE,
    RR_NEW_ACTIVE_NODE, 
    RR_CLOSE, 
    RR_READY
    };
    byte op;
    byte nodeId;
    byte status;
    int  size;
    struct { 
    int updateCount;
    int offs;
    } page;
};

struct RecoveryRequest { 
    dbFile*   file;
    int       nodeId;
    int       nPages;
    int*      updateCounters;
};
#endif


class dbFile { 
  protected:
#ifdef _WIN32
    HANDLE fh;
    HANDLE mh;
#else
#ifdef USE_SYSV_SHARED_MEMORY
    dbSharedMemory shmem;
#endif
    int    fd;
#endif
    char*  sharedName;
    char*  mmapAddr;
    size_t mmapSize;

  public:
    enum { 
    ok = 0
    };
    //
    // Create backup file
    //
    int    create(char const* name, bool noBuffering = true);
    //
    // Open database file and create file mapping object 
    //
    int    open(char const* fileName, char const* sharedName,
        bool readonly, size_t initSize, bool replicationSupport);
    
    void*  getAddr() const { return mmapAddr; }
    size_t getSize() const { return mmapSize; } 
    int    setSize(size_t size, char const* sharedName, bool initialize = true);
    int    flush(bool physical = false);
    int    close();
    int    erase();
    int    write(void const* ptr, size_t& writtenBytes, size_t size);
    int    read(void* ptr, size_t& readBytes, size_t size);
    bool   write(void const* ptr, size_t size);

    static char* errorText(int code, char* buf, size_t bufSize);

#if defined(NO_MMAP) || defined(REPLICATION_SUPPORT)
    void markAsDirty(size_t pos, size_t size) { 
    size_t page = pos >> dbModMapBlockBits;
    size_t last = (pos + size + dbModMapBlockSize - 1) >> dbModMapBlockBits;
        assert(int(last >> 5) <= pageMapSize);
    while (page < last) { 
        pageMap[page >> 5] |= 1 << (page & 31);
        page += 1;
    }
    }

  private:
    int* pageMap;
    int  pageMapSize;
    int  pageSize;
  public:
    int  updateCounter;

#ifdef REPLICATION_SUPPORT
    int*      currUpdateCount;
    int*      diskUpdateCount;
    byte*     rootPage;
    bool      doSync;
    bool      closing;

    dbReplicatedDatabase* db;

    int       getUpdateCountTableSize();
    int       getMaxPages(); 

    dbMutex   replCS;
    dbMutex   syncCS;

    dbThread     syncThread;
    dbLocalEvent syncEvent;
    dbLocalEvent recoveredEvent;
    int          nRecovered;

    static int   dbSyncTimeout; // milliseconds

#ifdef _WIN32
    HANDLE    cfh;
    HANDLE    cmh;
#else
    int       cfd;
#endif

    static void thread_proc startSyncToDisk(void* arg);
    static void thread_proc startRecovery(void* arg);


    void doRecovery(int nodeId, int* updateCounters, int nPages);

    void syncToDisk();
    void startSync();
    void stopSync();

  public:
    void configure(dbReplicatedDatabase* db) { 
    this->db = db;
    }

    bool updatePages(int nodeId, size_t pos, int updateCount, int size);
    bool concurrentUpdatePages(int nodeId, size_t pos, int updateCount, int size);
    void recovery(int nodeId, int* updateCounters, int nPages);
#endif

#else
    void markAsDirty(size_t, size_t) {}
#endif

    dbFile();
};


#endif

