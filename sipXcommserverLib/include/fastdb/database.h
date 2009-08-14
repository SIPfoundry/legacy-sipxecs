//-< DATABASE.H >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 23-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Database management
//-------------------------------------------------------------------*--------*

#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "class.h"
#include "reference.h"
#include "file.h"

BEGIN_FASTDB_NAMESPACE

/**
 * Default size of memory mapping object for the database (bytes)
 */
#ifdef DISKLESS_CONFIGURATION
// In diskless confiuration database can not be reallocated
const size_t dbDefaultInitDatabaseSize = 32*1024*1024;
#else
const size_t dbDefaultInitDatabaseSize = 1024*1024;
#endif

/**
 * Default initial index size (number of objects)
 */
const size_t dbDefaultInitIndexSize = 512*1024;

/**
 * Quantum of extension of allocated memory
 */
const size_t dbDefaultExtensionQuantum = 4*1024*1024;

/**
 * Maximal number of threads which can be spawned to perform parallel sequentila search
 */
const unsigned dbMaxParallelSearchThreads = 64;

const int dbDefaultParallelScanThreshold = 1000;
const int dbDefaultPollInterval = 10*1000; // milliseconds
const int dbWaitReadyTimeout = 60*1000; // milliseconds
const int dbWaitStatusTimeout = 60*1000; // milliseconds
const int dbRecoveryConnectionAttempts = 3;
const int dbStartupConnectionAttempts = 60;
const int dbReplicationWriteTimeout = 60*1000; // milliseconds
const int dbMaxAsyncRecoveryIterations = 1000;

/**
 * Internal objects tags
 */
enum dbInternalObject {
    dbTableRow,
    dbPageObjectMarker,
    dbTtreeMarker,
    dbTtreeNodeMarker,
    dbHashTableMarker,
    dbHashTableItemMarker,
    dbRtreeMarker,
    dbRtreePageMarker,

    dbInternalObjectMarker = 7 // mask for internals object markers
};

const offs_t dbFreeHandleMarker = (offs_t)1 << (sizeof(offs_t)*8 - 1);

const size_t dbAllocationQuantumBits = 4;
const size_t dbAllocationQuantum = 1 << dbAllocationQuantumBits;
const size_t dbPageBits = 12;
const size_t dbPageSize = 1 << dbPageBits;
const size_t dbIdsPerPage = dbPageSize / sizeof(oid_t);
const size_t dbHandlesPerPage = dbPageSize / sizeof(offs_t);
const size_t dbHandleBits = 1 + sizeof(offs_t)/4; // log(sizeof(offs_t))
const size_t dbBitmapSegmentBits = dbPageBits + 3 + dbAllocationQuantumBits;
const size_t dbBitmapSegmentSize = 1 << dbBitmapSegmentBits;
const size_t dbBitmapPages = 1 << (dbDatabaseOffsetBits-dbBitmapSegmentBits);
const size_t dbDirtyPageBitmapSize = 1 << (dbDatabaseOidBits-dbPageBits+dbHandleBits-3);
const size_t dbDefaultSelectionLimit = 2000000000;

const int    dbBMsearchThreshold = 512;

const char   dbMatchAnyOneChar = '_';
const char   dbMatchAnySubstring = '%';

const int    dbMaxReaders = 64; // maximal number of readers concurrently accessing the database

/**
 * Predefined object identifiers
 */
enum dbPredefinedIds {
    dbInvalidId,
    dbMetaTableId,
    dbBitmapId,
    dbFirstUserId = dbBitmapId + dbBitmapPages
};

/**
 * Database header
 */
class dbHeader {
  public:
    offs_t size;  // database file size
    int4   curr;  // current root
    int4   dirty; // database was not closed normally
    int4   initialized; // database is initilaized
#if dbDatabaseOffsetBits > 32 && defined(ALIGN_HEADER)
    int4   pad;
#endif
    struct {
        offs_t index;           // offset to object index
        offs_t shadowIndex;     // offset to shadow index
        oid_t  indexSize;       // size of object index
        oid_t  shadowIndexSize; // size of object index
        oid_t  indexUsed;       // used part of the index
        oid_t  freeList;        // L1 list of free descriptors
    } root[2];

    int4 majorVersion;
    int4 minorVersion;
    int4 mode;

    enum {
        MODE_OID_64        = 0x01,
        MODE_OFFS_64       = 0x02,
        MODE_AUTOINCREMENT = 0x04,
        MODE_RECTANGLE_DIM = 0x08
    };

    int getVersion() {
        return majorVersion*100 + minorVersion;
    }

    bool isCompatible();
    static int getCurrentMode();
};

union  dbSynthesizedAttribute;
struct dbInheritedAttribute;
class dbDatabaseThreadContext;
class dbAnyCursor;
class dbQuery;
class dbExprNode;

struct dbMemoryStatistic {
    offs_t used;
    offs_t free;
    offs_t nHoles;
    offs_t minHoleSize;
    offs_t maxHoleSize;
    size_t nHolesOfSize[dbDatabaseOffsetBits];
};

class FixedSizeAllocator
{
    struct Hole {
        Hole*  next;
        offs_t offs;
    };

    size_t minSize;
    size_t maxSize;
    size_t quantum;
    size_t nChains;
    size_t bufSize;
    Hole** chains;
    Hole*  vacant;
    Hole*  holes;

  public:
    size_t hits;
    size_t faults;
    size_t retries;

    FixedSizeAllocator();
    ~FixedSizeAllocator();

    void init(size_t minSize, size_t maxSize, size_t quantum, size_t bufSize);
    void reset();

    offs_t allocate(size_t size) {
        if (size - minSize <= maxSize - minSize) {
            size_t i = (size - minSize + quantum - 1) / quantum;
            Hole* hole = chains[i];
            if (hole != NULL) {
                hits += 1;
                chains[i] = hole->next;
                hole->next = vacant;
                vacant = hole;
                return hole->offs;
            }
            faults += 1;
        }
        return 0;
    }

    bool free(offs_t offs, size_t size) {
        if (vacant != NULL && size - minSize <= maxSize - minSize) {
            size_t i = (size - minSize + quantum - 1) / quantum;
            Hole* hole = vacant;
            vacant = hole->next;
            hole->next = chains[i];
            chains[i] = hole;
            hole->offs = offs;
            return true;
        }
        return false;
    }
};


class dbMonitor {
  public:
    sharedsem_t sem;
    sharedsem_t mutatorSem;
    int  nReaders;
    int  nWriters;
    int  nConcurrentWriters;
    int  nWaitReaders;
    int  nWaitWriters;
    int  waitForUpgrade;
    int  forceCommitCount;
    int  backupInProgress;
    int  uncommittedChanges;

    int  curr;             // copy of header->root, used to allow read access
                           // to the database during transaction commit
    offs_t size; // database size

    int  commitInProgress;
    int  concurrentTransId;

    unsigned lastDeadlockRecoveryTime;

    int  version;
    int  users;

    dbProcessId ownerPid;

    dbDatabaseThreadContext*  delayedCommitContext;     // save context of delayed transaction

    int4 dirtyPagesMap[dbDirtyPageBitmapSize/4];

    int  sharedLockOwner[dbMaxReaders];
    int  exclusiveLockOwner;
    int  clientId;
    int  upgradeId;

    int  modified;

#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
    struct {
        oid_t head;
        oid_t tail;
    } sessionFreeList[2];
#endif
};

/**
 * Double linked list
 */
class FASTDB_DLL_ENTRY dbL2List {
  public:
    dbL2List* next;
    dbL2List* prev;

    void link(dbL2List* elem) {
        elem->prev = this;
        elem->next = next;
        next = next->prev = elem;
    }
    void unlink() {
        next->prev = prev;
        prev->next = next;
        next = prev = this;
    }
    bool isEmpty() {
        return next == this;
    }
    void reset() {
        next = prev = this;
    }
    dbL2List() {
        next = prev = this;
    }
    ~dbL2List() {
        unlink();
    }
};

class dbVisitedObject {
  public:
    dbVisitedObject* next;
    oid_t            oid;

    dbVisitedObject(oid_t oid, dbVisitedObject* chain) {
        this->oid = oid;
        next = chain;
    }
};

#ifdef AUTO_DETECT_PROCESS_CRASH
struct dbWatchDogContext : dbL2List {
    dbThread    thread;
    dbWatchDog  watchDog;
    int         clientId;
    dbDatabase* db;
    dbMutex*    mutex;
};
#endif

template<class T>
class dbHArray;

typedef unsigned (*dbHashFunction)(byte* key, int type, int keylen);

/**
 * Database class
 */
class FASTDB_DLL_ENTRY dbDatabase {
    friend class dbSelection;
    friend class dbAnyCursor;
    friend class dbHashTable;
    friend class dbQuery;
    friend class dbTtree;
    friend class dbTtreeNode;
    friend class dbRtree;
    friend class dbRtreePage;
    friend class dbParallelQueryContext;
    friend class dbServer;
    friend class dbColumnBinding;
    friend class dbUserFunctionArgument;
    friend class dbAnyContainer;
    friend class dbFile;
    friend class dbCLI;
    friend class GiSTdb;

#ifdef HAS_TEMPLATE_FRIENDS
    template<class T>
    friend class dbHArray;
#else
    friend class dbAnyHArray;
#endif

  public:
    /**
     * Open database
     * @param databaseName database name
     * @param fielName path to the database file
     *   (if null, then file name daatbaseName + ".fdb" will be used)
     * @param waitLockTimeoutMsec timeout for waiting locks, by default disabled
     * @param commitDelaySec delayed commit timeout, by default disabled
     * @return <code>true</code> if database was successfully opened
     */
    bool open(char const* databaseName,
              char const* fileName = NULL,
              time_t waitLockTimeoutMsec = INFINITE,
              time_t commitDelaySec = 0);

    enum dbAccessType {
        dbReadOnly,
        dbAllAccess,
        dbConcurrentRead,
        dbConcurrentUpdate
    };

    /**
     * Structure to specify database open parameters
     */
    struct OpenParameters {
        /**
         * Database name
         */
        char const* databaseName;

        /**
         * Database file path
         */
        char const* databaseFilePath;

        /**
         * Transaction commit delay
         */
        time_t  transactionCommitDelay;

        /**
         * Deadlock detection timeout (after expiration of this timeout a lokc is revoked)
         */
        time_t waitLockTimeoutMsec;

        /**
         * Database access type
         */
        dbAccessType accessType;

        /**
         * Initial database file size
         */
        size_t initSize;

        /**
         * Quantum for extending memory allocation bitmap
         */
        size_t extensionQuantum;

        /**
         * Initial database index size
         */
        size_t initIndexSize;

        /**
         * Concurrency level for sequential search and sort operations
         */
        int nThreads;

        /**
         * Threshold for amount of deallocated space after which allocation bitmap is
         * scanned from the very beginning reusing deallocated object
         */
        offs_t freeSpaceReuseThreshold;

        /**
         * Minimal number of records in the table when performing sequential search in parallel makes sense
         */
        int parallelScanThreshold;

        /**
         * Replicated database node id
         */
        int nodeId;

        /**
         * Replicated database node addresses
         */
        char** nodeAddresses;

        /**
         * Number of nodes in replicasted database
         */
        int nNodes;

        /**
         * Interval of polling nodes
         */
        int pollInterval; // milliseconds

        /**
         * Timeout of waiting confirmation from standby node by new master node when it becomes active at startup
         */
        int waitReadyTimeout; // milliseconds

        /**
         * Timeout of requesting status of other nodes during startup
         */
        int waitStatusTimeout; // milliseconds

        /**
         * Maximal number of attempts to establish with other nodes during recovery
         */
        int recoveryConnectionAttempts;

        /**
         * Maximal number of attempts to establish with other nodes at startup
         */
        int startupConnectionAttempts;

        /**
         * Timeout of writing to replication node.
         * If write can not be completed within specified timeout, then node is considered to be dead
         * and connection is hanged up.
         */
        int replicationWriteTimeout;

        /**
         * Maximal number of asynchronous recovery iterations. If due to permanent updates of master database
         * recovery can not be completed within specified number of iterations, then synchronous recovery is performed
         * (master will not perform any transaction commits until the end of recovery).
         */
        int maxAsyncRecoveryIterations;

        OpenParameters() {
            databaseName = NULL;
            databaseFilePath = NULL;
            transactionCommitDelay = 0;
            waitLockTimeoutMsec = INFINITE;
            accessType = dbAllAccess;
            extensionQuantum = dbDefaultExtensionQuantum;
            initSize = dbDefaultInitDatabaseSize;
            initIndexSize = dbDefaultInitIndexSize;
            nThreads = 1;
            freeSpaceReuseThreshold = dbDefaultExtensionQuantum;
            nodeId = 0;
            nodeAddresses = NULL;
            nNodes = 0;
            parallelScanThreshold = dbDefaultParallelScanThreshold;
            pollInterval = dbDefaultPollInterval;
            waitReadyTimeout = dbWaitReadyTimeout;
            waitStatusTimeout = dbWaitStatusTimeout;
            recoveryConnectionAttempts = dbRecoveryConnectionAttempts;
            startupConnectionAttempts = dbStartupConnectionAttempts;
            replicationWriteTimeout = dbReplicationWriteTimeout;
            maxAsyncRecoveryIterations = dbMaxAsyncRecoveryIterations;
        }
    };

    /**
     * Open database with parameters defined in OpenParameters structure
     * @param params parameters for openning database
     * @return <code>true</code> if database was successfully opened
     */
    bool open(OpenParameters& params);

    /**
     * Close database
     */
    void close();

    /**
     * Commit transaction
     */
    void commit();

    /**
     * Release all locks hold by transaction allowing other clients to proceed
     * but do not flush changes to the disk
     */
    void precommit();

    /**
     * Rollback transaction
     */
    void rollback();

    /**
     * Schedule backup
     * @param fileName path to backup file. If name ends with '?', then
     * each backup willbe placed in seprate file with '?' replaced with current timestamp
     * @param periodSec preiod of performing backups in seconds
     */
    void scheduleBackup(char const* fileName, time_t periodSec);

    /**
     * Attach current thread to the database. This method should be executed
     * for all threads except one which opened the database.
     */
    void attach();

    /**
     * Set transaction context for the current thread. Using this method allows to share the same transaction
     * between different threads
     * @param ctx transaction context which will be associated with the current thread
     */
    void attach(dbDatabaseThreadContext* ctx);


    enum DetachFlags {
        COMMIT          = 1,
        DESTROY_CONTEXT = 2
    };
    /**
     * Detach thread from the database.
     * @param flags mask of DetachFlags COMMIT and DESTROY_CONTEXT
     */
    void detach(int flags = COMMIT|DESTROY_CONTEXT);

    enum dbLockType {
        dbSharedLock,
        dbExclusiveLock,
        dbCommitLock
    };

    /**
     * Exclusivly lock the database.
     */
    void lock(dbLockType lock = dbExclusiveLock) { beginTransaction(lock); }

    /**
     * Perform backup to the file with specified name
     * @param file path to the backup file
     * @param comactify if true then databae will be compactificated during backup -
     * i.e. all used objects will be placed together without holes; if false then
     * backup is performed by just writting memory mapped object to the backup file.
     * @return whether backup was succeseful or not
     */
    bool backup(char const* file, bool compactify);

    /**
     * Perform backup to the specified file
     * @param file opened file to path to the backup file. This file will not be closed after
     * backup completion.
     * @param comactify if true then databae will be compactificated during backup -
     * i.e. all used objects will be placed together without holes; if false then
     * backup is performed by just writting memory mapped object to the backup file.
     * @return whether backup was succeseful or not
     */
    bool backup(dbFile* file, bool compactify);

    /**
     * Assign table to the database
     * @param desc table descriptor
     */
    void assign(dbTableDescriptor& desc) {
        assert(((void)"Table is not yet assigned to the database",
                desc.tableId == 0));
        desc.db = this;
        desc.fixedDatabase = true;
    }

    /**
     * Set concurrency level for sequential search and sort operations.
     * By default, FastDB tries to detect number of CPUs in system and create
     * the same number of threads.
     * @param nThreads maximal number of threads to be created for
     * perfroming cincurrent sequential search and sorting.
     */
    void setConcurrency(unsigned nThreads);

    /**
     * Get size allocated in the database since open
     * @return delta between size of allocated and deallocated data
     */
    long getAllocatedSize() { return allocatedSize; }

    /**
     * Get size of the database file
     * @return database file size
     */
    long getDatabaseSize() { return header->size; }

    /**
     * Get number of threads accessing database in shared mode (readonly)
     * @return number of granted shared locks
     */
    int getNumberOfReaders() {
        return monitor->nReaders;
    }

    /**
     * Get number of threads accessing database in exclusiveh mode (for update)
     * @return number of granted exclusive locks (can be either 0 either 1)
     */
    int getNumberOfWriters() {
        return monitor->nWriters;
    }

    /**
     * Get number of threads blocked while starting read-only transaction
     * @return number of threads which shared lock request was blocked
     */
    int getNumberOfBlockedReaders() {
        return monitor->nReaders;
    }

    /**
     * Get number of threads blocked while starting update transaction
     * @return number of threads which exclusive lock request was blocked
     */
    int getNumberOfBlockedWriters() {
        return monitor->nWriters;
    }

    /**
     * Get number of processes attached to the database
     * @return number of processes openned the database
     */
    int getNumberOfUsers() {
        return monitor->users;
    }

    /**
     * Enable deletion of columns from the table when correspondent fields
     * are renamed from class descriptor. By default it is switched of
     * and database allows to delete fields only from empty table (to prevent
     * unindented loose of data).
     * @param enabled true to enable column deletion in non empty tables
     */
    void allowColumnsDeletion(bool enabled = true) {
        confirmDeleteColumns = enabled;
    }

    /**
     * Prepare query. This method can be used for explicit compilation of query and
     * it's validation
     * @param cursor result set
     * @param query query expression
     * @return <code>true</code> if query is successfully compiled, <code>false</code> othgerwise
     */
    bool prepareQuery(dbAnyCursor* cursor, dbQuery& query);

    enum dbErrorClass {
        NoError,
        QueryError,
        ArithmeticError,
        IndexOutOfRangeError,
        DatabaseOpenError,
        FileError,
        OutOfMemoryError,
        Deadlock,
        NullReferenceError,
        LockRevoked,
        FileLimitExeeded,
        InconsistentInverseReference,
        DatabaseReadOnly
    };
    static char const* const errorMessage[];
    typedef void (*dbErrorHandler)(int error, char const* msg, int msgarg, void* context);

    /**
     * Set error handler. Handler should be no-return function which perform stack unwind.
     * @param newHandler new error handler
     * @return previous handler
     */
    dbErrorHandler setErrorHandler(dbErrorHandler newHandler, void* errorHandlerContext = NULL);

    /**
     * Error handler.
     * It can be redifined by application to implement application specific error handling.
     * @param error class of the error
     * @param msg error message
     * @param arg optional argument
     */
    virtual void handleError(dbErrorClass error, char const* msg = NULL,
                             int arg = 0);

    /**
     * Insert record in the database
     * @param table table descriptor
     * @param ref   [out] pointer to the references where ID of created object will be stored
     * @param record pointer to the transient object to be inserted in the table
     */
    void insertRecord(dbTableDescriptor* table, dbAnyReference* ref,
                      void const* record);

    /**
     * Check if database is opened
     */
    bool isOpen() const { return opened; }


    /**
     * Check if current transaction is committed. Used mostly for debugging purposes.
     */
    bool isCommitted();

    /**
     * Check if thread was attached to the database. Used mostly for debugging purposes.
     */
    bool isAttached();

    /**
     * Check if current transaction is updating database
     */
    bool isUpdateTransaction();

    /**
     * Get database version
     */
    int  getVersion();

    /**
     * Specify database file size limit. Attempt to exeed this limit cause database error.
     * @param limit maximal file size in bytes
     */
    void setFileSizeLimit(size_t limit) {
        fileSizeLimit = limit;
    }

#ifdef FUZZY_CHECKPOINT
    /**
     * Specify maximal length of qeueue of write requests during fuzzy checkpoint.
     * If this limit is reached, commit is blocked until some of the requests are proceeded
     * @param nPages maximal number of pages in write queue
     */
    void setFuzzyCheckpointBuffer(size_t nPages) {
        file.setCheckpointBufferSize(nPages);
    }
#endif

#ifndef NO_MEMBER_TEMPLATES
    /**
     * Insert record in the database
     * @param record transient object to be insrted in the database
     * @return reference to the created object
     */
    template<class T>
    dbReference<T> insert(T const& record) {
        dbReference<T> ref;
        insertRecord(lookupTable(&T::dbDescriptor), &ref, &record);
        return ref;
    }
#endif
    /**
     * Find cloned table desciptor assigned to this database
     * @param des static unassigned table descriptor
     * @return clone of this table descriptor assigned to this databae or NULL
     * if not found.
     */
    dbTableDescriptor* lookupTable(dbTableDescriptor* desc);

    /**
     * Get information about state of database memory
     * @param stat placeholder for memory statistic
     */
    void getMemoryStatistic(dbMemoryStatistic& stat);

    /**
     * Initialize fixed size allocator
     * @param minSize minial object size allocated by this allocator
     * @param maxSize maximal object size allocated by this allocator
     * @param quantum difference in size between fixed size allocator chains
     * @param bufSize maximal number of free elements hold by this allocator
     */
    void setFixedSizeAllocator(size_t minSize, size_t maxSize, size_t quantum, size_t bufSize) {
        fixedSizeAllocator.init(minSize, maxSize, quantum, bufSize);
    }

    enum dbThreadMode {
        dbNotUsePthreads,
        dbUsePthreads
    };

    enum dbReplicationMode {
        dbReplicated,
        dbStandalone
    };

    /**
     * Database constructor
     * @param type access type: <code>dbDatabase::dbReadOnly</code> or <code>dbDatabase::dbAllAcces</code>
     * @param dbInitSize initial size of the database. If FastDB is compiled with
     * DISKLESS_CONFIGURATION option, then in this parameter <B>MAXIMAL</B> size of the
     * database should be specified (in this mode database can not be reallocated)
     * @param dbExtensionQuantum quantum for extending memory allocation bitmap
     * @param dbInitIndexSize initial index size (objects)
     * @param nThreads concurrency level for sequential search and sort operations
     * @see setConcurrency(unsigned nThreads)
     */
    dbDatabase(dbAccessType type = dbAllAccess,
               size_t dbInitSize = dbDefaultInitDatabaseSize,
               size_t dbExtensionQuantum = dbDefaultExtensionQuantum,
               size_t dbInitIndexSize = dbDefaultInitIndexSize,
               int nThreads = 1
               // Do not specify the following parameter - them are used only for checking
               // that application and FastDB library were built with the
               // same compiler options (-DNO_PTHREADS and -REPPLICATION_SUPPORT)
               // Mismached parameters should cause linker error
#ifdef NO_PTHREADS
               , dbThreadMode threadMode = dbNotUsePthreads
#endif
#ifdef REPLICATION_SUPPORT
               , dbReplicationMode replicationMode = dbReplicated
#endif
               );
    /**
     * Database detructor
     */
    virtual ~dbDatabase();

    dbAccessType accessType;
    size_t initSize;
    size_t extensionQuantum;
    size_t initIndexSize;
    offs_t freeSpaceReuseThreshold;

  protected:
    static size_t internalObjectSize[];

    dbThreadPool threadPool;

    FixedSizeAllocator fixedSizeAllocator;

    dbThreadContext<dbDatabaseThreadContext> threadContext;

    byte*     baseAddr;         // base address of database file mapping
    dbHeader* header;           // database header information
    offs_t*   currIndex;        // current database object index
    offs_t*   index[2];
    unsigned  parThreads;
    bool      modified;

    size_t    currRBitmapPage;  //current bitmap page for allocating records
    size_t    currRBitmapOffs;  //offset in current bitmap page for allocating
                                //unaligned records
    size_t    currPBitmapPage;  //current bitmap page for allocating page objects
    size_t    currPBitmapOffs;  //offset in current bitmap page for allocating
                                //page objects
    struct dbLocation {
        offs_t      pos;
        size_t      size;
        dbLocation* next;
    };
    dbLocation* reservedChain;

    char*     databaseName;
    int       databaseNameLen;
    char*     fileName;
    int       version;

    size_t    committedIndexSize;
    size_t    currIndexSize;
    oid_t     updatedRecordId;

    unsigned  waitLockTimeout;

    size_t    fileSizeLimit;
    bool      uncommittedChanges;

    dbHashFunction hashFunction;

    dbFile                    file;
    dbSharedObject<dbMonitor> shm;
    dbGlobalCriticalSection   cs;
    dbGlobalCriticalSection   mutatorCS;
    dbInitializationMutex     initMutex;
    dbSemaphore               writeSem;
    dbSemaphore               readSem;
    dbSemaphore               upgradeSem;
    dbEvent                   backupCompletedEvent;
    dbMonitor*                monitor;

    dbTableDescriptor*        tables;

    int*                      bitmapPageAvailableSpace;
    bool                      opened;

    offs_t                    allocatedSize;
    offs_t                    deallocatedSize;

    time_t                    commitDelay;
    time_t                    commitTimeout;
    time_t                    commitTimerStarted;

    dbMutex                   delayedCommitStartTimerMutex;
    dbMutex                   delayedCommitStopTimerMutex;
    dbLocalEvent              delayedCommitStartTimerEvent;
    dbEvent                   delayedCommitStopTimerEvent;
    dbLocalEvent              commitThreadSyncEvent;
    bool                      delayedCommitEventsOpened;

    dbMutex                   backupMutex;
    dbLocalEvent              backupInitEvent;
    char*                     backupFileName;
    time_t                    backupPeriod;
    bool                      stopDelayedCommitThread;

    dbThread                  backupThread;
    dbThread                  commitThread;

    int                       accessCount;

    dbL2List                  threadContextList;
    dbMutex                   threadContextListMutex;

    dbErrorHandler            errorHandler;
    void*                     errorHandlerContext;
    int                       schemeVersion;
    dbVisitedObject*          visitedChain;

    bool                      confirmDeleteColumns;

    int                       maxClientId;
    int                       selfId;
#ifdef AUTO_DETECT_PROCESS_CRASH
    dbWatchDog                selfWatchDog;
    dbL2List                  watchDogThreadContexts;
    dbMutex*                  watchDogMutex;
#endif

    unsigned parallelScanThreshold;


   /**
     * Loads all class descriptors. This method should be used SubSQL and any other apllication
     * which is should work with ANY database file.
     * @return metatable descriptor
     */
    dbTableDescriptor* loadMetaTable();

    void cleanup(dbInitializationMutex::initializationStatus status, int step);

    void delayedCommit();
    void backupScheduler();

    static void thread_proc delayedCommitProc(void* arg) {
        ((dbDatabase*)arg)->delayedCommit();
    }

    static void thread_proc backupSchedulerProc(void* arg) {
        ((dbDatabase*)arg)->backupScheduler();
    }

    virtual bool isReplicated();

#ifdef AUTO_DETECT_PROCESS_CRASH
    /**
     * Revoke lock of crashed client
     * @param clientId ID of crashed client
     */
    void revokeLock(int clientId);

    /**
     * Watch dog thread procedure
     * @param ctx watch dog context
     */
    static void watchDogThread(dbWatchDogContext* ctx);

    /**
     * Start watch dog threads for new clients
     */
    void startWatchDogThreads();

    /**
     * Add information about lock owner
     */
    void addLockOwner();

    /**
     * Remove information about lock owner
     */
    void removeLockOwner(int clientId);
#endif

    /**
     * Commit transaction
     * @param ctx thread context
     */
    void commit(dbDatabaseThreadContext* ctx);

    /**
     * Restore consistency of table list of rows (last record should contain null reference
     * in next field). This method is used during recovery after crash and during rollback.
     */
    void restoreTablesConsistency();

    /**
     * Check if OID corresponds to the valid object
     * @param oid inspected OID
     * @return whether OID is valid or not
     */
    bool isValidOid(oid_t oid) {
        if (oid < dbFirstUserId || oid >= currIndexSize) {
            return false;
        }
        return !(currIndex[oid]&(dbFreeHandleMarker|dbInternalObjectMarker));
    }


    /**
     * Get table row
     * @param oid object indentifier
     * @return object with this oid
     */
    dbRecord* getRow(oid_t oid) {
        assert(!(currIndex[oid]&(dbFreeHandleMarker|dbInternalObjectMarker)));
        return (dbRecord*)(baseAddr + currIndex[oid]);
    }

    /**
     * Prepare for row insertion or update. If record with such OID
     * not exists or it is first time when it was changed during this transaction or
     * size of recrod is changed, than new record is alocated in the database.
     * Otherwisepointer to existed recordis returned.
     * @param oid object indetifier
     * @param newSize size of new object
     * @return pointer inside database where object should should be stored
     */
    dbRecord* putRow(oid_t oid, size_t newSize);

    /**
     * Prepare for object update ithout changing its size
     * @param oid object indetifier
     * @return pointer inside database where object should should be stored
     */
    dbRecord* putRow(oid_t oid) {
        if (oid < committedIndexSize && index[0][oid] == index[1][oid]) {
            size_t size = getRow(oid)->size;
            size_t pageNo = oid/dbHandlesPerPage;
            monitor->dirtyPagesMap[pageNo >> 5] |= 1 << (pageNo & 31);
            cloneBitmap(currIndex[oid], size);
            allocate(size, oid);
        }
        return (dbRecord*)(baseAddr + currIndex[oid]);
    }

    /**
     * Get record by OID
     * @param oid object identifier
     * @return pointer to the record inside database
     */
    byte* get(oid_t oid) {
        return baseAddr + (currIndex[oid] & ~dbInternalObjectMarker);
    }

    /**
     * Prepare for update of internal object.
     * @param oid internal object identifier
     * @return pointer to the record inside database
     */
    byte* put(oid_t oid) {
        if (oid < committedIndexSize && index[0][oid] == index[1][oid]) {
            offs_t offs = currIndex[oid];
            size_t size = internalObjectSize[offs & dbInternalObjectMarker];
            size_t pageNo = oid/dbHandlesPerPage;
            monitor->dirtyPagesMap[pageNo >> 5] |= 1 << (pageNo & 31);
            allocate(size, oid);
            cloneBitmap(offs & ~dbInternalObjectMarker, size);
        }
        return baseAddr + (currIndex[oid] & ~dbInternalObjectMarker);
    }

    /**
     * Check whether query is prefix search: "'key' like field+'%'"
     * @param cursor result set
     * @param expr evaluated expression
     * @param andExpr if not null, then it is used as filter to all records selected by
     * index search
     * @param indexedFile [out] used to return information about which field was used
     * to perfrom index search and so order in which selected records are sorted.
     * If this order is the same as requested by "order by" clause, then no extra sorting
     * is needed.
     * @return true if search was performed using indeices,  false if index is not applicable
     * and sequential search is required
     */
    bool isPrefixSearch(dbAnyCursor* cursor,
                        dbExprNode* expr, dbExprNode* andExpr,
                        dbFieldDescriptor* &indexedField);

    /**
     * Check whether search can be performed using indices
     * @param cursor result set
     * @param expr evaluated expression
     * @param andExpr if not null, then it is used as filter to all records selected by
     * index search
     * @param indexedFile [out] used to return information about which field was used
     * to perfrom index search and so order in which selected records are sorted.
     * If this order is the same as requested by "order by" clause, then no extra sorting
     * is needed.
     * @return true if search was performed using indeices,  false if index is not applicable
     * and sequential search is required
     */
    bool isIndexApplicable(dbAnyCursor* cursor,
                           dbExprNode* expr, dbExprNode* andExpr,
                           dbFieldDescriptor* &indexedField);

    /**
     * Check whether expression can be evaluated unsing index.
     * If index is applicable, than index search is performed and result
     * is stored in the cursor.
     * @param cursor result set
     * @param expr evaluated expression
     * @param andExpr if not null, then it is used as filter to all records selected by
     * index search
     * @return true if expression was evaluated using index, false if index is not applicable
     * and sequential search is required
     */
    bool isIndexApplicable(dbAnyCursor* cursor,
                           dbExprNode* expr, dbExprNode* andExpr);

    /**
     * If query predicate contains operands from other tables (accessed by references)
     * and inverse references exists, then FastDB performs
     * indexed search in referenced table and then go back using inverse referenced to
     * query table. followInverseReference method performs this backward traversal of inverse
     * references.
     * @param expr evaluated expression
     * @param andExpr if not null, then it is used as filter to all records selected by
     * index search
     * @param cursor cursor to collect selected records
     * @param iref OID of the selected records in referenced table
     */
    bool followInverseReference(dbExprNode* expr, dbExprNode* andExpr,
                                dbAnyCursor* cursor, oid_t iref);

    /**
     * Check if there is inverse reference in the table rerefrenced from search predicate.
     * @param expr evaluated expression
     * @param nExistsClause number of exists clauses in search wrapping this expression
     * @return true if inverse reference(s) exists and it is possible to perform backward
     * traversal
     */
    bool existsInverseReference(dbExprNode* expr, int nExistsClauses);

    /**
     * Execute expression. This method is most frequently recursivly called during
     * evaluation of search predicate.
     * @param expr expression to be executed
     * @param iattr inherited attributes - attributes passed top-down
     * (information like cursor, current record, ...)
     * @param sattr synthesized attribute - sttributes passed down-top
     * (value of expression)
     */
    static void _fastcall execute(dbExprNode* expr,
                                  dbInheritedAttribute& iattr,
                                  dbSynthesizedAttribute& sattr);

    /**
     * Evaluate epression. This method initialie initiainherited attributes and invoke
     * execute method
     * @param expr expression to be evaluated
     * @param oid OID of the inspected record
     * @param seaqched table
     * @param cursor result set
     * @return true if this record match search condition, false otherwise
     */
    bool evaluate(dbExprNode* expr, oid_t oid, dbTable* table, dbAnyCursor* cursor);

    /**
     * Select all records from the table
     * @param cursor result set
     */
    void select(dbAnyCursor* cursor);

    /**
     * Execute select query
     * @param cursor result set
     * @param query query expression
     */
    void select(dbAnyCursor* cursor, dbQuery& query);

    /**
     * Perform table traverse: execute queries with "start from (follow by)" clause
     * @param cursor result set
     * @param query  query expression
     */
    void traverse(dbAnyCursor* cursor, dbQuery& query);

    /**
     * Update record
     * @param oid record identifier
     * @param table descriptor of the table to which record belongs
     * @param record updated image of the record
     */
    void update(oid_t oid, dbTableDescriptor* table, void const* record);

    /**
     * Remove record from the database
     * @param table descriptor of the table to which record belongs
     * @param oid record identifier
     */
    void remove(dbTableDescriptor* table, oid_t oid);

    /**
     * Allocate object in the database
     * e@param size size of alocated object
     * @param oid if oid is not 0, then allocated region position is stored in correcpondent
     * cell of object index (needed for allocation of bitmap pages)
     * @return position of allcoated region
     */
    offs_t allocate(size_t size, oid_t oid = 0);

    /**
     * Deallocate region
     * @param pos start position of region
     * @param size of region
     */
    void deallocate(offs_t pos, size_t size);

    /**
     * Checks whther allocated size is greater than size of databae file and recreate
     * memory mapping object with larger size n the last case
     * @param size allocated size
     */
    void extend(offs_t size);

    /**
     * Clone memory allocation bitmap for region [pos, pos+size)
     * @param pos start of region
     * @param size size of region
     */
    void cloneBitmap(offs_t pos, size_t size);

    /**
     * Allocate object identifier(s)
     * @param number of allocated object indentifiers
     * @return object idenitifer (in case if n greater than 1, all n subsequent OIDs are
     * allocated and first one is returned
     */
    oid_t allocateId(int n = 1);

    /**
     * Free object identifier(s)
     * @param oid deallocated object identifer (or first of n deallocated subsequent identifiers
     * if n greater than 1)
     * @param number of allocated object indentifiers
     */
    void freeId(oid_t oid, int n = 1);

    /**
     * Update record in in all active cursors if it this record is checnged in the database
     * @param oid object indentifier of checnged record
     * @param removed true if record was removed
      */
    void updateCursors(oid_t oid, bool removed = false);

    /**
     * Perform database recovery after fault
     */
    void recovery();

    /**
     * Check if program works with correct version of memory mapped object (if memory mapped
     * object is reallocated by some client, its version number is incremented, so
     * all other client will be able to notice it and also reallocate their memory
     * mapping objects.
     * @return true if memory mapping object was successfully reallocated or no reallocation
     * is needed at all
     */
    bool checkVersion();

    /**
     * Allocate internal object
     * @param market internal object tag
     * @return oid of allocated object
     */
    oid_t allocateObject(dbInternalObject marker) {
        oid_t oid = allocateId();
        offs_t pos = allocate(internalObjectSize[marker]) + marker;
        currIndex[oid] = pos;
        return oid;
    }

    /**
     * Allocate record
     * @param tableId object identifier of the table
     * @param size size of the created record
     * as table descriptor in the database
     */
    oid_t allocateRow(oid_t tableId, size_t size)
    {
        oid_t oid = allocateId();
        allocateRow(tableId, oid, size);
        return oid;
    }

    /**
     * Allocate record with specified OID
     * @param tableId object identifier of the table
     * @param oid record OID
     * @param size size of the created record
     * as table descriptor in the database
     */
    void allocateRow(oid_t tableId, oid_t oid, size_t size);

    /**
     * Delete row from the table
     * @param tableId OID of record with table descriptor
     * @param oid identifier of deleted record
     */
    void freeRow(oid_t tableId, oid_t oid);

    /**
     * Free internal object
     */
    void freeObject(oid_t oid);

    /**
     * Cleanup compiled query
     */
    static void deleteCompiledQuery(dbExprNode* tree);

    /**
     * Start database transaction
     * @param modify if it is update or read-only rtansaction
     * @return true if version of memory mapping object is not obsolete and reallocation
     * is not possible
     */
    bool beginTransaction(dbLockType);

    /**
     * End transaction
     */
    void endTransaction() {
        endTransaction(threadContext.get());
    }

    /**
     * End transaction with specified thread context
     * @param ctx thread context
     */
    void endTransaction(dbDatabaseThreadContext* ctx);

    virtual void waitTransactionAcknowledgement();

    /**
     * Initialize database metatable (table containning information about all other tables
     * included metatable itself). This method is invoked during database initialzation.
     */
    void initializeMetaTable();

    /**
     * Load database scheme. This method loads table decriptors from database,
     * compare them with application classes, do necessary reformatting and save
     * update andnew table decriptor in database
     * @param alter if true then schema can be altered, otherwise there are some
     * other active clients working with this database so schema can not be altered
     */
    bool loadScheme(bool alter);

    /**
     * This method is invoked by SubSQL to complete table descriptors initialization
     * after loading of all table descriptoes from thr database
     * @return true if intertable relation consuistency is rpeservedm false otherwise
     */
    bool completeDescriptorsInitialization();

    /**
     * Reformat table according to new format
     * @param tableId OID of  changed tables
     * @param nw table descriptor
     */
    void reformatTable(oid_t tableId, dbTableDescriptor* desc);

    /**
     * Add new indices to the table.
     * @param alter if true than indices can be added, otherwise there are some other active
     * clients and adding new indices about which they will not know can lead to inconsistncy
     * @param desc new table descriptor
     * @return true if indices were succesfully added
     */
    bool addIndices(bool alter, dbTableDescriptor* desc);

    /**
     * Add new table to the database
     * @param desc - descriptor of new table
     * @return oid of created table descriptor record
     */
    oid_t addNewTable(dbTableDescriptor* desc);

    /**
     * Update database table descriptor
     * @param desc application table descriptor
     * @param tableId OID of recrods with database table descriptor
     */
    void updateTableDescriptor(dbTableDescriptor* desc, oid_t tableId);

    /**
     * Insert inverse reference. When reference or array of reference which is part of relation is updated
     * then reference to the updated record is inserted in inverse reference field of all
     * new referenced records (which were not referenced by this field  before update).
     * @param fd descriptor of updated field (inverse reference should exist for this field)
     * @param reverseId OID of updated record
     * @param targetId OID of record referenced by this field
     */
    void insertInverseReference(dbFieldDescriptor* fd,
                                oid_t reverseId, oid_t targetId);

    /**
     * Remove inverse references to the removed record
     * @param desc descriptor of table from  which record is removed
     * @param oid  OID of removed record
     */
    void removeInverseReferences(dbTableDescriptor* desc, oid_t oid);

    /**
     * Remove inverse reference. When reference or array of reference which is part of relation is updated
     * then reference to the updated record is removed from inverse reference field of all
     * referenced records which are not reference any more from by this field.
     * @param fd descriptor of updated field (inverse reference should exist for this field)
     * @param reverseId OID of updated record
     * @param targetId OID of record referenced by this field
     */
    void removeInverseReference(dbFieldDescriptor* fd,
                                oid_t reverseId, oid_t targetId);

    /**
     * Delete table from the database
     * @param desc table descriptor
     */
    void deleteTable(dbTableDescriptor* desc);

    /**
     * Delete all table records
     * @param desc table descriptor
     */
    void dropTable(dbTableDescriptor* desc);

    /**
     * Create T-Tree index for the field
     * @param fd field descriptor
     */
    void createIndex(dbFieldDescriptor* fd);

    /**
     * Create hash table for the field
     * @param fd field descriptor
     */
    void createHashTable(dbFieldDescriptor* fd);

    /**
     * Drop T-Tree index for the field
     * @param fd field descriptor
     */
    void dropIndex(dbFieldDescriptor* fd);

    /**
     * Drop hash table for the field
     * @param fd field descriptor
     */
    void dropHashTable(dbFieldDescriptor* fd);

    /**
     * Link table to the database table list
     * @param table table descriptor
     * @param tableId OID of record containing database table descriptor
     */
    void linkTable(dbTableDescriptor* table, oid_t tableId);

    /**
     * Unlink table from the database tables list
     * @param table table descriptor
     */
    void unlinkTable(dbTableDescriptor* table);

    /**
     * Check if location is reserved
     * @param pos start position of the location
     * @param size location size
     * @return true id location was reserved
     */
    bool wasReserved(offs_t pos, size_t size);

    /**
     * Mark location as reserved. This method is used by allocator to protect hole
     * located in memory allocation bitmap, from been used by recursuve call of allocator (needed to clone
     * bitmap pages).
     * @param location [out] local structure describing location.
     * @param pos start position of the location
     * @param size location size
     */
    void reserveLocation(dbLocation& location, offs_t pos, size_t size);

    /**
     * Remove location from list of reserved locations. It is done after location is marked
     * as occupied in bitmap.
     */
    void commitLocation();

    /**
     * Find table using symbol name
     * @param name symbol table entry (returned by dbSymbolTable::add method)
     * @return table descriptor or <code>NULL</code> if not found
     */
    dbTableDescriptor* findTable(char const* name);

    /**
     * Find table by name. This method get symbol for specified name and call <code>findTable</code>
     * method.
     * @param name name of table
     * @return table descriptor or <code>NULL</code> if not found
     */
    dbTableDescriptor* findTableByName(char const* name);

    /**
     * Get list of tables attached to the database
     * @return list of tables attached to the database
     */
    dbTableDescriptor* getTables() {
        return tables;
    }

    /**
     * Mark database as been modified
     */
    void setDirty();

    /**
     * Check if given location is free
     * @param pos object offset
     * @param objBitSize object size i quantums
     */
    bool isFree(offs_t pos, int objBitSize);

    /**
     * Set in memory allocation bitmap bits corresponding to the object
     * @param pos object offset
     * @param objBitSize object size i quantums
     */
    void markAsAllocated(offs_t pos, int objBitSize);
};


#ifdef REPLICATION_SUPPORT

#include "sockio.h"

class FASTDB_DLL_ENTRY dbConnection {
 public:
    socket_t*    reqSock;
    socket_t*    respSock;
    dbLocalEvent statusEvent;
    dbLocalEvent readyEvent;
    dbLocalEvent useEvent;
    dbLocalEvent committedEvent;
    dbMutex      writeCS;
    int          useCount;
    int          waitUseEventFlag;
    int          waitStatusEventFlag;
    int          status;
    int          updateCounter;

    dbConnection() {
        readyEvent.open();
        useEvent.open();
        statusEvent.open();
        committedEvent.open();
        useCount = 0;
        waitUseEventFlag = 0;
        waitStatusEventFlag = 0;
        status = 0;
        reqSock = respSock = NULL;
    }
    ~dbConnection() {
        readyEvent.close();
        useEvent.close();
        statusEvent.close();
        committedEvent.close();
        delete reqSock;
        delete respSock;
    }
};

class FASTDB_DLL_ENTRY dbReplicatedDatabase : public dbDatabase {
    friend class dbFile;
  protected:
    char**        serverURL;
    int           nServers;
    int           id;
    dbConnection* con;

    enum NodeStatus {
        ST_OFFLINE,  // node is not available
        ST_ONLINE,   // node is available
        ST_ACTIVE,   // primary node is running, replicating changes
        ST_STANDBY,  // standby node receives changes from primary node
        ST_RECOVERED // node is recovered after the fault
    };

    dbLocalEvent  startEvent;
    dbLocalEvent  recoveredEvent;
    dbMutex       startCS;
    dbMutex       commitCS;
    fd_set        inputSD;
    int           nInputSD;

    int           activeNodeId;
    dbMutex       sockCS;
    socket_t*     acceptSock;
    dbThread      readerThread;

    int pollInterval;
    int waitReadyTimeout;
    int waitStatusTimeout;
    int recoveryConnectionAttempts;
    int startupConnectionAttempts;
    int replicationWriteTimeout;
    int maxAsyncRecoveryIterations;

    static void thread_proc startReader(void* arg);

    void waitTransactionAcknowledgement();

    void reader();
    virtual bool isReplicated();

  public:
    void deleteConnection(int nodeId);
    void lockConnection(int nodeId);
    void unlockConnection(int nodeId);
    void changeActiveNode();
    void addConnection(int nodeId, socket_t* s);
    bool writeReq(int nodeId, ReplicationRequest const& hdr,
                  void* body = NULL, size_t bodySize = 0);
    bool writeResp(int nodeId, ReplicationRequest const& hdr);

    bool open(char const* databaseName, char const* fileName,
              int id, char* servers[], int nServers);
    bool open(OpenParameters& params);
    virtual void close();

    int getNumberOfOnlineNodes();

    dbReplicatedDatabase(dbAccessType type = dbAllAccess,
                         size_t dbInitSize = dbDefaultInitDatabaseSize,
                         size_t dbExtensionQuantum = dbDefaultExtensionQuantum,
                         size_t dbInitIndexSize = dbDefaultInitIndexSize,
                         int nThreads = 1);
};
#endif

template<class T>
dbReference<T> insert(T const& record) {
    dbReference<T> ref;
    T::dbDescriptor.getDatabase()->insertRecord(&T::dbDescriptor, &ref, &record);
    return ref;
}

#ifdef NO_MEMBER_TEMPLATES
template<class T>
dbReference<T> insert(dbDatabase& db, T const& record) {
    dbReference<T> ref;
    db.insertRecord(db.lookupTable(&T::dbDescriptor), &ref, &record);
    return ref;
}
#endif

/**
 * Search contrext used to pass information about search parameters to T-Tree and Hash table index implementations
 */
class dbSearchContext {
  public:
    dbDatabase*     db;
    dbExprNode*     condition;
    dbAnyCursor*    cursor;
    char*           firstKey;
    int             firstKeyInclusion;
    char*           lastKey;
    int             lastKeyInclusion;
    int             type;
    int             sizeofType;
    int             prefixLength;
    dbUDTComparator comparator;
    int             offs;
    int             probes;
};


END_FASTDB_NAMESPACE

#endif
