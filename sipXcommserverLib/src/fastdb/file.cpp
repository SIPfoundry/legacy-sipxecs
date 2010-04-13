//-< FILE.CPP >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// System dependent implementation of mapped on memory file
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "stdtp.h"
#include "file.h"
#include "database.h"

BEGIN_FASTDB_NAMESPACE


#if FUZZY_CHECKPOINT
struct dbWriteQuery {
    dbWriteQuery* next;
    size_t        pos;
    char          page[dbPageSize];
};

#define DEFAULT_QUEUE_LENGTH_LIMIT 16*1024

class dbFileWriter {
  private:
    dbLocalSemaphore event;
    dbMutex          mutex;
    dbWriteQuery*    last;
    dbWriteQuery*    first;
    dbWriteQuery*    free;
    int              queueLength;
    int              queueLengthLimit;
    int              underflow;
    int              overflow;
    int              running;
    dbThread         thread;
    dbFile*          file;

    static void thread_proc writeLoop(void* arg) {
        ((dbFileWriter*)arg)->write();
    }

    void write() {
        mutex.lock();
        while (true) {
            dbWriteQuery* query = first;
            if (query != NULL) {
                queueLength -= 1;
                if (overflow) {
                    overflow = false;
                    event.signal();
                }
                first = query->next;
                mutex.unlock();
                file->write(query->pos, query->page, dbPageSize);
                mutex.lock();
                query->next = free;
                free = query;
            } else {
                if (!running) {
                    break;
                }
                underflow = true;
                event.wait(mutex);
            }
        }
        mutex.unlock();
    }

  public:
    dbFileWriter(dbFile* file) {
        this->file = file;
        queueLength = 0;
        queueLengthLimit = DEFAULT_QUEUE_LENGTH_LIMIT;
        event.open();
        first = last = NULL;
        free = NULL;
        overflow = underflow = false;
        running = true;
        thread.create(&writeLoop, this);
    }

    void setQueueLimit(size_t limit) {
        queueLengthLimit = limit;
    }

    ~dbFileWriter() {
        mutex.lock();
        running = false;
        event.signal();
        mutex.unlock();
        thread.join();
        dbWriteQuery* query = first;
        while (query != NULL) {
            dbWriteQuery* next = query->next;
            delete query;
            query = next;
        }
        event.close();
    }

    void put(size_t pos, void* page, size_t size) {
        char* beg = (char*)page;
        char* end = beg + size;
        while (beg < end) {
            put(pos, beg);
            pos += dbPageSize;
            beg += dbPageSize;
        }
    }

    void put(size_t pos, void* page) {
        mutex.lock();
        while (queueLength >= queueLengthLimit) {
            overflow = true;
            event.wait(mutex);
        }
        dbWriteQuery* query = free;
        if (query == NULL) {
            query = new dbWriteQuery();
        } else {
            free = query->next;
        }
        if (first == NULL) {
            first = query;
        } else {
            last->next = query;
        }
        queueLength += 1;
        last = query;
        query->next = NULL;
        query->pos = pos;
        memcpy(query->page, page, dbPageSize);
        if (underflow) {
            underflow = false;
            event.signal();
        }
        mutex.unlock();
    }
};

void dbFile::setCheckpointBufferSize(size_t nPages)
{
    writer->setQueueLimit(nPages);
}

#endif

dbFile::dbFile()
{
    sharedName = NULL;
    mmapAddr = NULL;
    mmapSize = 0;
    readonly = false;

#ifdef REPLICATION_SUPPORT
    currUpdateCount = NULL;
    diskUpdateCount = NULL;
    rootPage = NULL;
    db = NULL;
#endif
}

dbFile::~dbFile()
{
}

#if defined(REPLICATION_SUPPORT) || defined(NO_MMAP)
const size_t dbMaxSyncSegmentSize = 128*1024 / dbModMapBlockSize;
#endif

#ifdef REPLICATION_SUPPORT

#include "database.h"

int dbFile::dbSyncTimeout = 1000; // one second

bool dbFile::updatePages(socket_t* s, size_t pos, int pageUpdateCounter, int size)
{
    if (pos + size > mmapSize) {
        size_t newSize = pos + size > mmapSize*2 ? pos + size : mmapSize*2;
        setSize(newSize, sharedName);
        ((dbHeader*)mmapAddr)->size = newSize;
        db->baseAddr = (byte*)mmapAddr;
        db->header = (dbHeader*)mmapAddr;
        db->version = db->monitor->version += 1;
    }
    if (s->read(mmapAddr + pos, size)) {
        int pageNo = pos >> dbModMapBlockBits;
        if (updateCounter < pageUpdateCounter) {
            updateCounter = pageUpdateCounter;
        }
        while (size > 0) {
            currUpdateCount[pageNo++] = pageUpdateCounter;
            size -= dbModMapBlockSize;
        }
        return true;
    }
    return false;
}

bool dbFile::concurrentUpdatePages(socket_t* s, size_t pos, int pageUpdateCounter, int size)
{
    if (pos + size > mmapSize) {
        size_t newSize = pos + size > mmapSize*2 ? pos + size : mmapSize*2;
        TRACE_IMSG(("Extend database from %ld to %ld\n", (long)mmapSize, (long)newSize));
        db->beginTransaction(dbDatabase::dbCommitLock);
        setSize(newSize, sharedName);
        ((dbHeader*)mmapAddr)->size = newSize;
#ifdef PROTECT_DATABASE
        protect(0, mmapSize);
#endif
        db->baseAddr = (byte*)mmapAddr;
        db->header = (dbHeader*)mmapAddr;
        db->version = db->monitor->version += 1;
        db->endTransaction();
    }
    if (pos == 0 && size <= pageSize) {
        if (!s->read(rootPage, size)) {
            return false;
        }
        if (((dbHeader*)rootPage)->curr != ((dbHeader*)mmapAddr)->curr) {
            TRACE_MSG(("Commit transaction at replica\n"));
            db->beginTransaction(dbDatabase::dbCommitLock);
#ifdef PROTECT_DATABASE
            unprotect(pos, size);
#endif
            memcpy(mmapAddr, rootPage, size);
            // now readers will see updated data
            db->monitor->curr = ((dbHeader*)mmapAddr)->curr;
            db->endTransaction();
#ifdef SYNCHRONOUS_REPLICATION
            ReplicationRequest rr;
            rr.op = ReplicationRequest::RR_COMMITTED;
            rr.size = 0;
            s->write(&rr, sizeof rr);
#endif
        } else {
#ifdef PROTECT_DATABASE
            unprotect(pos, size);
#endif
            memcpy(mmapAddr, rootPage, size);
        }
    } else {
#ifdef PROTECT_DATABASE
        unprotect(pos, size);
#endif
        if (!s->read(mmapAddr + pos, size)) {
#ifdef PROTECT_DATABASE
            protect(pos, size);
#endif
            return false;
        }
        assert(((dbHeader*)mmapAddr)->curr == db->monitor->curr);
    }
#ifdef PROTECT_DATABASE
    protect(pos, size);
#endif
    int pageNo = pos >> dbModMapBlockBits;
    if (updateCounter < pageUpdateCounter) {
        updateCounter = pageUpdateCounter;
    }
    while (size > 0) {
        currUpdateCount[pageNo++] = pageUpdateCounter;
        size -= dbModMapBlockSize;
    }
    return true;
}

int dbFile::getUpdateCountTableSize() {
    int nPages = mmapSize >> dbModMapBlockBits;
    while (--nPages >= 0 && diskUpdateCount[nPages] == 0);
    return nPages + 1;
}

int dbFile::getMaxPages()
{
    return 1 << (dbDatabaseOffsetBits - dbModMapBlockBits);
}


void dbFile::startSync()
{
#ifndef DISKLESS_CONFIGURATION
    doSync = true;
    closing = false;
    syncEvent.reset();
    syncThread.create(startSyncToDisk, this);
#endif
}

void dbFile::stopSync()
{
#ifndef DISKLESS_CONFIGURATION
    doSync = false;
    syncEvent.signal();
    syncThread.join();
#endif
}

void thread_proc dbFile::startSyncToDisk(void* arg)
{
    ((dbFile*)arg)->syncToDisk();
}

void thread_proc dbFile::startRecovery(void* arg)
{
    RecoveryRequest* rr = (RecoveryRequest*)arg;
    rr->file->doRecovery(rr->nodeId, rr->updateCounters, rr->nPages);
    {
        dbCriticalSection cs(rr->file->replCS);
        if (--rr->file->nRecovered == 0) {
            rr->file->recoveredEvent.signal();
        }
    }
    delete rr;
}

void dbFile::recovery(int nodeId, int* updateCounters, int nPages)
{
    RecoveryRequest* rr = new RecoveryRequest;
    rr->nodeId = nodeId;
    rr->updateCounters = updateCounters;
    rr->nPages = nPages;
    rr->file = this;
    {
        dbCriticalSection cs(replCS);
        if (nRecovered++ == 0) {
            recoveredEvent.reset();
        }
    }
    dbThread recoveryThread;
    recoveryThread.create(startRecovery, rr);
    recoveryThread.setPriority(dbThread::THR_PRI_HIGH);
}


int dbFile::sendChanges(int nodeId, int* updateCounters, int nPages)
{
    ReplicationRequest rr;
    int maxUpdateCount = 0;
    TRACE_MSG(("Database synchronization: mmapSize=%ld\n", mmapSize));
    size_t i, j, n;

    for (i = 0, j = 0, n = mmapSize >> dbModMapBlockBits; i < n; i++) {
        if (updateCounters[i] > currUpdateCount[i]) {
            updateCounters[i] = 0;
        } else {
            if (updateCounters[i] > maxUpdateCount) {
                maxUpdateCount = updateCounters[i];
            }
        }
        if (i > j && (currUpdateCount[i] <= updateCounters[i] || i-j >= dbMaxSyncSegmentSize
                      || currUpdateCount[i] != currUpdateCount[j]))
        {
            rr.op = ReplicationRequest::RR_RECOVER_PAGE;
            rr.nodeId = nodeId;
            rr.size = (i-j)*dbModMapBlockSize;
            rr.page.offs = j << dbModMapBlockBits;
            rr.page.updateCount = currUpdateCount[j];
            TRACE_MSG(("Send segment [%lx, %ld]\n", rr.page.offs, rr.size));
            if (!db->writeReq(nodeId, rr, mmapAddr + rr.page.offs, rr.size)) {
                return -1;
            }
            j = i;
        }
        if (i >= (size_t)nPages || currUpdateCount[i] > updateCounters[i]) {
            if (currUpdateCount[i] > maxUpdateCount) {
                maxUpdateCount = currUpdateCount[i];
            }
            updateCounters[i] = currUpdateCount[i];
        } else {
            j = i + 1;
        }
    }
    if (i != j) {
        rr.op = ReplicationRequest::RR_RECOVER_PAGE;
        rr.nodeId = nodeId;
        rr.size = (i-j)*dbModMapBlockSize;
        rr.page.offs = j << dbModMapBlockBits;
        rr.page.updateCount = currUpdateCount[j];
        TRACE_MSG(("Send segment [%lx, %ld]\n", rr.page.offs, rr.size));
        if (!db->writeReq(nodeId, rr, mmapAddr + rr.page.offs, rr.size)) {
            return -1;
        }
    }
    return maxUpdateCount;
}

void dbFile::completeRecovery(int nodeId)
{
    ReplicationRequest rr;
    dbTrace("Complete recovery of node %d\n", nodeId);
    rr.op = ReplicationRequest::RR_STATUS;
    rr.nodeId = nodeId;
    db->con[nodeId].status = rr.status = dbReplicatedDatabase::ST_STANDBY;
    for (int i = 0, n = db->nServers; i < n; i++) {
        if (db->con[i].status != dbReplicatedDatabase::ST_OFFLINE && i != db->id) {
            db->writeReq(i, rr);
        }
    }
}

void dbFile::doRecovery(int nodeId, int* updateCounters, int nPages)
{
    int maxUpdateCount;
    memset(updateCounters+nPages, 0, (getMaxPages() - nPages)*sizeof(int));

    if (db->con[nodeId].reqSock == NULL) {
        char buf[256];
        socket_t* s = socket_t::connect(db->serverURL[nodeId],
                                        socket_t::sock_global_domain,
                                        db->recoveryConnectionAttempts);
        if (!s->is_ok()) {
            s->get_error_text(buf, sizeof buf);
            dbTrace("Failed to establish connection with node %d: %s\n",
                    nodeId, buf);
            delete s;
            return;
        }
        ReplicationRequest rr;
        rr.op = ReplicationRequest::RR_GET_STATUS;
        rr.nodeId = db->id;
        if (!s->write(&rr, sizeof rr) || !s->read(&rr, sizeof rr)) {
            s->get_error_text(buf, sizeof buf);
            dbTrace("Connection with node %d is broken: %s\n",
                    nodeId, buf);
            delete s;
            return;
        }
        if (rr.op != ReplicationRequest::RR_STATUS && rr.status != dbReplicatedDatabase::ST_STANDBY) {
            dbTrace("Unexpected response from standby node %d: code %d status %d\n",
                     nodeId, rr.op, rr.status);
            delete s;
            return;
        } else {
            db->addConnection(nodeId, s);
        }
    }

    for (int i = 0; i < db->maxAsyncRecoveryIterations; i++) {
        {
            dbCriticalSection cs(syncCS);
            maxUpdateCount = sendChanges(nodeId, updateCounters, nPages);
            if (maxUpdateCount < 0) {
                delete[] updateCounters;
                return;
            }
        }
        {
            dbCriticalSection cs(replCS);
            if (maxUpdateCount == updateCounter) {
                delete[] updateCounters;
                completeRecovery(nodeId);
                return;
            }
        }
    }
    {
        dbTrace("Syncronouse recovery of node %d\n", nodeId);
        dbCriticalSection cs1(syncCS);
        dbCriticalSection cs2(replCS);
        maxUpdateCount = sendChanges(nodeId, updateCounters, nPages);
        delete[] updateCounters;
        if (maxUpdateCount >= 0) {
            assert(maxUpdateCount == updateCounter);
            completeRecovery(nodeId);
        }
    }
}


#endif

bool dbFile::write(void const* buf, size_t size)
{
    size_t writtenBytes;
    bool result = write(buf, writtenBytes, size) == ok && writtenBytes == size;
    return result;
}

#ifdef _WIN32

class OS_info : public OSVERSIONINFO {
  public:
    OS_info() {
        dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(this);
    }
};

static OS_info osinfo;

#define BAD_POS 0xFFFFFFFF // returned by SetFilePointer and GetFileSize


int dbFile::erase()
{
    return ok;
}


#ifdef PROTECT_DATABASE
void dbFile::protect(size_t pos, size_t size)
{
    PDWORD oldProtect;
    bool rc = VirtualProtect(mmapAddr + pos, DOALIGN(size, pageSize), PAGE_READONLY, &oldProtect);
    assert(rc);
}

void dbFile::unprotect(size_t pos, size_t size)
{
    PDWORD oldProtect;
    bool rc = VirtualProtect(mmapAddr + pos, DOALIGN(size, pageSize), PAGE_READWRITE, &oldProtect);
    assert(rc);
}
#endif


int dbFile::open(char const* fileName, char const* sharedName, bool readonly,
                 size_t initSize, bool replicationSupport)
{
    int status;
    size_t fileSize;
    this->readonly = readonly;
#ifndef DISKLESS_CONFIGURATION
#if defined(_WINCE) && !defined(NO_MMAP)
    fh = CreateFileForMapping
#else
    fh = CreateFile
#endif
        (W32_STRING(fileName),
         readonly ? GENERIC_READ : (GENERIC_READ|GENERIC_WRITE),
         FILE_SHARE_READ | FILE_SHARE_WRITE,
         FASTDB_SECURITY_ATTRIBUTES,
         readonly ? OPEN_EXISTING : OPEN_ALWAYS,
#ifdef _WINCE
         FILE_ATTRIBUTE_NORMAL
#else
         FILE_FLAG_RANDOM_ACCESS
#ifdef NO_MMAP
         |FILE_FLAG_NO_BUFFERING
#endif
#if 0 // not needed as we do explicit flush ???
         |FILE_FLAG_WRITE_THROUGH
#endif
#endif
         , NULL);
    if (fh == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }
    DWORD highSize;
    fileSize = GetFileSize(fh, &highSize);
    if (fileSize == BAD_POS && (status = GetLastError()) != ok) {
        CloseHandle(fh);
        return status;
    }
    assert(highSize == 0);

    mmapSize = fileSize;

    this->sharedName = new char[strlen(sharedName) + 1];
    strcpy(this->sharedName, sharedName);

    if (!readonly && fileSize == 0) {
        mmapSize = initSize;
    }
#else
    fh = INVALID_HANDLE_VALUE;
    this->sharedName = NULL;
    mmapSize = fileSize = initSize;
#endif
#if defined(NO_MMAP)
    if (fileSize < mmapSize && !readonly) {
        if (SetFilePointer(fh, mmapSize, NULL, FILE_BEGIN) != mmapSize || !SetEndOfFile(fh)) {
            status = GetLastError();
            CloseHandle(fh);
            return status;
        }
    }
    mmapAddr = (char*)VirtualAlloc(NULL, mmapSize, MEM_COMMIT|MEM_RESERVE,
                                   PAGE_READWRITE);

#ifdef DISKLESS_CONFIGURATION
    if (mmapAddr == NULL)
#else
    DWORD readBytes;
    if (mmapAddr == NULL
        || !ReadFile(fh, mmapAddr, fileSize, &readBytes, NULL) || readBytes != fileSize)
#endif
    {
        status = GetLastError();
        if (fh != INVALID_HANDLE_VALUE) {
            CloseHandle(fh);
        }
        return status;
    }
    memset(mmapAddr+fileSize, 0, mmapSize - fileSize);
    mh = NULL;
#else
    mh = CreateFileMapping(fh, FASTDB_SECURITY_ATTRIBUTES, readonly ? PAGE_READONLY : PAGE_READWRITE,
                           0, mmapSize, W32_STRING(sharedName));
    status = GetLastError();
    if (mh == NULL) {
        if (fh != INVALID_HANDLE_VALUE) {
            CloseHandle(fh);
        }
        return status;
    }
    mmapAddr = (char*)MapViewOfFile(mh, readonly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (mmapAddr == NULL) {
        status = GetLastError();
        CloseHandle(mh);
        if (fh != INVALID_HANDLE_VALUE) {
            CloseHandle(fh);
        }
        return status;
    }
    if (status != ERROR_ALREADY_EXISTS && mmapSize > fileSize)
        //      && osinfo.dwPlatformId != VER_PLATFORM_WIN32_NT)
    {
        // Windows 95 doesn't initialize pages
        memset(mmapAddr+fileSize, 0, mmapSize - fileSize);
    }
#endif

#if defined(NO_MMAP) || defined(REPLICATION_SUPPORT)
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    pageSize = systemInfo.dwPageSize;
    pageMapSize = (mmapSize + dbModMapBlockSize*32 - 1) >> (dbModMapBlockBits + 5);
    pageMap = new int[pageMapSize];
    memset(pageMap, 0, pageMapSize*sizeof(int));
#endif

#if defined(REPLICATION_SUPPORT)
    db = NULL;
    int nPages = getMaxPages();
    currUpdateCount = new int[nPages];

    if (replicationSupport) {
        char* cFileName = new char[strlen(fileName) + 5];
        strcat(strcpy(cFileName, fileName), ".cnt");

#ifdef DISKLESS_CONFIGURATION
        cfh = INVALID_HANDLE_VALUE;
#else
        cfh = CreateFile(cFileName, GENERIC_READ|GENERIC_WRITE,
                         0, NULL, OPEN_ALWAYS,
                         FILE_FLAG_RANDOM_ACCESS|FILE_FLAG_WRITE_THROUGH,
                         NULL);
        delete[] cFileName;
        if (cfh == INVALID_HANDLE_VALUE) {
            status = errno;
            return status;
        }
#endif
        cmh = CreateFileMapping(cfh, NULL, PAGE_READWRITE, 0,
                                nPages*sizeof(int), NULL);
        status = GetLastError();
        if (cmh == NULL) {
            CloseHandle(cfh);
            return status;
        }
        diskUpdateCount = (int*)MapViewOfFile(cmh, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (diskUpdateCount == NULL) {
            status = GetLastError();
            CloseHandle(cmh);
            CloseHandle(cfh);
            return status;
        }
        rootPage = dbMalloc(pageSize);
        int maxCount = 0;
        for (int i = 0; i < nPages; i++) {
            int count = diskUpdateCount[i];
            currUpdateCount[i] = count;
            if (count > maxCount) {
                maxCount = count;
            }
        }
        updateCounter = maxCount;
        nRecovered = 0;
        recoveredEvent.open(true);
        syncEvent.open();
        startSync();
    }
#endif

#ifdef FUZZY_CHECKPOINT
    writer = new dbFileWriter(this);
#endif

    return ok;
}

bool dbFile::write(size_t pos, void const* ptr, size_t size)
{
    DWORD written;
    if (SetFilePointer(fh, pos, NULL, FILE_BEGIN) != pos ||
        !WriteFile(fh, ptr, size, &written, NULL)
        || written != (DWORD)size)
    {
        dbTrace("Failed to save page to the disk, position=%ld, size=%ld, error=%d\n",
                (long)pos, (long)size, GetLastError());
        return false;
    }
    return true;
}

#if defined(REPLICATION_SUPPORT)
void dbFile::syncToDisk()
{
    syncThread.setPriority(dbThread::THR_PRI_LOW);
    dbCriticalSection cs(syncCS);
    while (doSync) {
        size_t i, j, k, n;
        int maxUpdated = 0;
        for (i = 0, n = mmapSize >> dbModMapBlockBits; i < n;) {
            int updateCounters[dbMaxSyncSegmentSize];
            for (j=i; j < (mmapSize >> dbModMapBlockBits) && j-i < dbMaxSyncSegmentSize
                     && currUpdateCount[j] > diskUpdateCount[j]; j++)
            {
                updateCounters[j-i] = currUpdateCount[j];
            }
            if (i != j) {
                size_t pos = (i << dbModMapBlockBits) & ~(pageSize-1);
                size_t size = (((j-i) << dbModMapBlockBits) + pageSize - 1) & ~(pageSize-1);
#ifdef NO_MMAP
                write(pos, mmapAddr + pos, size);
#else
                FlushViewOfFile(mmapAddr + pos, size);
#endif
                for (k = 0; i < j; k++, i++) {
                    diskUpdateCount[i] = updateCounters[k];
                }
                maxUpdated = i;
            } else {
                i += 1;
            }
            if (!doSync) {
                return;
            }
        }
        if (maxUpdated != 0) {
            FlushViewOfFile(diskUpdateCount, maxUpdated*sizeof(int));
        }
        if (closing && maxUpdated == 0) {
            return;
        } else {
            syncEvent.wait(syncCS, dbSyncTimeout);
        }
    }
}
#endif

int dbFile::create(const char* name, bool noBuffering)
{
    fh = CreateFile(W32_STRING(name), GENERIC_READ|GENERIC_WRITE, 0, FASTDB_SECURITY_ATTRIBUTES, CREATE_ALWAYS,
                    (noBuffering ? FILE_FLAG_NO_BUFFERING : 0)|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (fh == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }
    mh = NULL;
    mmapAddr = NULL;
    sharedName = NULL;
    return ok;
}

int dbFile::read(void* buf, size_t& readBytes, size_t size)
{
    DWORD count;
    if (ReadFile(fh, buf, size, &count, NULL)) {
        readBytes = count;
        return ok;
    } else {
        readBytes = 0;
        return GetLastError();
    }
}

int dbFile::write(void const* buf, size_t& writtenBytes, size_t size)
{
    DWORD count;
    if (WriteFile(fh, buf, size, &count, NULL)) {
        writtenBytes = count;
        return ok;
    } else {
        writtenBytes = 0;
        return GetLastError();
    }
}


int dbFile::flush(bool physical)
{
#if defined(REPLICATION_SUPPORT)
    dbCriticalSection cs(replCS);
    if (db == NULL) {
        physical = true;
    }
    if (!physical) {
        updateCounter += 1;
    }
#endif
#if defined(REPLICATION_SUPPORT) || (defined(NO_MMAP) && !defined(DISKLESS_CONFIGURATION))
    int* map = pageMap;
    for (size_t i = 0, n = pageMapSize; i < n; i++) {
        if (map[i] != 0) {
            size_t pos = i << (dbModMapBlockBits + 5);
            unsigned mask = map[i];
            int count = 0;
            do {
                size_t size = 0;
                while ((mask & 1) == 0) {
                    pos += dbModMapBlockSize;
                    mask >>= 1;
                    count += 1;
                }
                while (true) {
                    do {
#ifdef REPLICATION_SUPPORT
                        if (!physical) {
                            currUpdateCount[(pos + size) >> dbModMapBlockBits] = updateCounter;
                        }
#endif
                        size += dbModMapBlockSize;
                        mask >>= 1;
                        count += 1;
                    } while ((mask & 1) != 0);
                    if (i+1 < n && count == 32 && size < dbMaxSyncSegmentSize*dbModMapBlockSize
                        && (map[i+1] & 1) != 0)
                    {
                        map[i] = 0;
                        mask = map[++i];
                        count = 0;
                    } else {
                        break;
                    }
                }
#if defined(REPLICATION_SUPPORT)
                if (db != NULL) {
                    if (!physical) {
                        for (int j = db->nServers; --j >= 0;) {
                            if (db->con[j].status == dbReplicatedDatabase::ST_STANDBY) {
                                ReplicationRequest rr;
                                rr.op = ReplicationRequest::RR_UPDATE_PAGE;
                                rr.nodeId = db->id;
                                rr.page.updateCount = updateCounter;
                                rr.page.offs = pos;
                                rr.size = size;
                                db->writeReq(j, rr, mmapAddr + pos, size);
                            }
                        }
                    }
                    pos += size;
                    continue;
                }
#endif
#ifndef DISKLESS_CONFIGURATION
#ifdef FUZZY_CHECKPOINT
                writer->put(pos, mmapAddr + pos, size);
#else
                if (!write(pos, mmapAddr + pos, size)) {
                    return GetLastError();
                }
#endif
#endif
                pos += size;
            } while (mask != 0);
            map[i] = 0;
        }
    }
#endif
#if !defined(NO_MMAP) && !defined(DISKLESS_CONFIGURATION) && !defined(REPLICATION_SUPPORT) && !defined(NO_FLUSH_ON_COMMIT)
    if (!FlushViewOfFile(mmapAddr, mmapSize)) {
        return GetLastError();
    }
#endif
    return ok;
}

int dbFile::setSize(size_t size, char const* sharedName, bool initialize)
{
#if defined(REPLICATION_SUPPORT)
    dbCriticalSection cs1(syncCS);
    dbCriticalSection cs2(replCS);
#endif
#ifdef DISKLESS_CONFIGURATION
    assert(false);
#else
#ifdef NO_MMAP
    char* newBuf = (char*)VirtualAlloc(NULL, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (newBuf == NULL) {
        return GetLastError();
    }
    if (SetFilePointer(fh, size, NULL, FILE_BEGIN) != size || !SetEndOfFile(fh)) {
        return GetLastError();
    }

    memcpy(newBuf, mmapAddr, mmapSize);
    VirtualFree(mmapAddr, 0, MEM_RELEASE);
    mmapAddr = newBuf;
    mmapSize = size;
#else
    if (!UnmapViewOfFile(mmapAddr) || !CloseHandle(mh)) {
        return GetLastError();
    }
    mh = CreateFileMapping(fh, FASTDB_SECURITY_ATTRIBUTES, readonly ? PAGE_READONLY : PAGE_READWRITE,
                           0, size, W32_STRING(sharedName));
    int status = GetLastError();
    if (mh == NULL) {
        printf("CreateFileMapping failed: %d\n", status);
        return status;
    }
    mmapAddr = (char*)MapViewOfFile(mh, readonly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (mmapAddr == NULL) {
        return GetLastError();
    }
    if (initialize && status != ERROR_ALREADY_EXISTS)
        //&& osinfo.dwPlatformId != VER_PLATFORM_WIN32_NT)
    {
        // Windows 95 doesn't initialize pages
        memset(mmapAddr+mmapSize, 0, size - mmapSize);
    }
    mmapSize = size;
#endif
#if defined(NO_MMAP) || defined(REPLICATION_SUPPORT)
    int newPageMapSize = (size + dbModMapBlockSize*32 - 1) >> (dbModMapBlockBits + 5);
    int* newPageMap = new int[newPageMapSize];
    memcpy(newPageMap, pageMap, pageMapSize*sizeof(int));
    memset(newPageMap + pageMapSize, 0,
           (newPageMapSize-pageMapSize)*sizeof(int));
    delete[] pageMap;
    pageMapSize = newPageMapSize;
    pageMap = newPageMap;
#endif

#endif
    return ok;
}

int dbFile::close()
{
    delete[] sharedName;
#if defined(REPLICATION_SUPPORT)
    if (db != NULL) {
        closing = true;
        stopSync();
        {
            dbCriticalSection cs(replCS);
            if (nRecovered != 0) {
                recoveredEvent.wait(replCS);
            }
        }
        syncEvent.close();
        recoveredEvent.close();
        UnmapViewOfFile(diskUpdateCount);
        CloseHandle(cmh);
        CloseHandle(cfh);
    }
    delete[] currUpdateCount;
    currUpdateCount = NULL;
    dbFree(rootPage);
    rootPage = NULL;
#endif
    if (mmapAddr != NULL) {
#if defined(NO_MMAP)
        int rc = flush();
        if (rc != ok) {
            return rc;
        }
#ifdef FUZZY_CHECKPOINT
        delete writer;
#endif
        VirtualFree(mmapAddr, 0, MEM_RELEASE);
        delete[] pageMap;
#else
        if (!UnmapViewOfFile(mmapAddr)) {
            return GetLastError();
        }
#if defined(REPLICATION_SUPPORT)
        delete[] pageMap;
#endif
#endif
    }
    if (mh != NULL) {
        if (!CloseHandle(mh)) {
            return GetLastError();
        }
    }
    return fh == INVALID_HANDLE_VALUE || CloseHandle(fh) ? ok : GetLastError();
}

char* dbFile::errorText(int code, char* buf, size_t bufSize)
{
#ifndef PHAR_LAP
#if defined(_WINCE) || defined(UNICODE)
    wchar_t cnvBuf[CNV_BUF_SIZE];
    int len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            code,
                            0,
                            cnvBuf,
                            CNV_BUF_SIZE-1,
                            NULL);
	cnvBuf[CNV_BUF_SIZE-1] = '\0';
	len = wcstombs(buf, cnvBuf, bufSize);
#else
    int len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            code,
                            0,
                            buf,
                            bufSize-1,
                            NULL);
#endif
    if (len == 0) {
        char errcode[64];
        sprintf(errcode, "unknown error code %u", code);
        strncpy(buf, errcode, bufSize-1);
    }
#else
    char errcode[64];
    sprintf(errcode, "unknown error code %u", code);
    strncpy(buf, errcode, bufSize-1);
#endif
    buf[bufSize-1] = '\0';
    return buf;
}

#else // Unix

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#ifndef NO_MMAP
#include <sys/mman.h>
#endif

#ifndef O_SYNC
#define O_SYNC  O_FSYNC
#endif

#ifndef O_DSYNC
#define O_DSYNC O_SYNC
#endif

#ifndef O_DIRECT
#define O_DIRECT 0
#endif


#ifdef PROTECT_DATABASE
void dbFile::protect(size_t pos, size_t size)
{
    int rc = mprotect(mmapAddr + pos, DOALIGN(size, pageSize), PROT_READ);
    assert(rc == 0);
}

void dbFile::unprotect(size_t pos, size_t size)
{
    int rc = mprotect(mmapAddr + pos, DOALIGN(size, pageSize), PROT_READ|PROT_WRITE);
    assert(rc == 0);
}
#endif

int dbFile::open(char const* name, char const*, bool readonly, size_t initSize, bool replicationSupport)
{
    this->readonly = readonly;
#if defined(USE_SYSV_SHARED_MEMORY) && defined(DISKLESS_CONFIGURATION)
    if (!shmem.open(name, initSize)) {
        return errno;
    }
    mmapSize = initSize;
    mmapAddr = shmem.get_base();
    fd = -1;
#else
#ifndef NO_MMAP
    int mmap_attr = MAP_SHARED;
#endif
    int status;
#ifdef DISKLESS_CONFIGURATION
#ifndef MAP_ANONYMOUS
    fd = ::open("/dev/zero", O_RDWR, 0);
#else
    fd = -1;
    mmap_attr |= MAP_ANONYMOUS;
#endif // MAP_ANONYMOUS
    mmapSize = initSize;
#else // DISKLESS_CONFIGURATION
    fd = ::open(name, readonly ? O_RDONLY : O_RDWR/*|O_DSYNC*/|O_CREAT
#ifdef NO_MMAP
                |O_DIRECT
#endif
                , 0666);
    if (fd < 0) {
        return errno;
    }
#if defined(__sun)
    directio(fd, DIRECTIO_ON);
#endif
    mmapSize = lseek(fd, 0, SEEK_END);
    if (!readonly && mmapSize == 0) {
        mmapSize = initSize;
        if (ftruncate(fd, mmapSize) != ok) {
            status = errno;
            if (fd >= 0) {
                ::close(fd);
            }
            return status;
        }
    }
#endif // DISKLESS_CONFIGURATION
#ifdef NO_MMAP
    size_t fileSize = mmapSize;
    if (!readonly && mmapSize < initSize) {
        mmapSize = initSize;
    }
#ifdef USE_SYSV_SHARED_MEMORY
    if (!shmem.open(name, mmapSize)) {
        status = errno;
        if (fd >= 0) {
            ::close(fd);
        }
        return status;
    }
    mmapAddr = shmem.get_base();
#else
    mmapAddr = (char*)valloc(mmapSize);
    if (mmapAddr == NULL) {
        status = errno;
        if (fd >= 0) {
            ::close(fd);
        }
        return status;
    }
#endif
    lseek(fd, 0, SEEK_SET);
    if ((size_t)::read(fd, mmapAddr, fileSize) != fileSize) {
#ifdef USE_SYSV_SHARED_MEMORY
        shmem.close();
#else
        free(mmapAddr);
#endif
        mmapAddr = NULL;
        status = errno;
        if (fd >= 0) {
            ::close(fd);
        }
        return status;
    }
#else  // NO_MMAP
    mmapAddr = (char*)mmap(NULL, mmapSize,
                           readonly ? PROT_READ : PROT_READ|PROT_WRITE,
                           mmap_attr, fd, 0);
    if (mmapAddr == (char*)-1) {
        status = errno;
        mmapAddr = NULL;
        if (fd >= 0) {
            ::close(fd);
        }
        return status;
    }
#endif // NO_MMAP
#endif // USE_SYSV_SHARED_MEMORY && DISKLESS_CONIFIGURATION
#if defined(NO_MMAP) || defined(REPLICATION_SUPPORT)
    pageSize = getpagesize();
    pageMapSize = (mmapSize + dbModMapBlockSize*32 - 1) >> (dbModMapBlockBits + 5);
    pageMap = new int[pageMapSize];
    memset(pageMap, 0, pageMapSize*sizeof(int));
#endif
#if defined(REPLICATION_SUPPORT)
    db = NULL;
    int nPages = getMaxPages();
    currUpdateCount = new int[nPages];

    if (replicationSupport) {
        char* cFileName = new char[strlen(name) + 5];
        strcat(strcpy(cFileName, name), ".cnt");
        int uc_mmap_attr = MAP_SHARED;
#ifndef DISKLESS_CONFIGURATION
        cfd = ::open(cFileName, O_RDWR|O_DSYNC|O_CREAT, 0666);
        delete[] cFileName;
        if (cfd < 0) {
            return errno;
        }
        if (ftruncate(cfd, nPages*sizeof(int)) != ok) {
            status = errno;
            ::close(cfd);
            return status;
        }
#else
#ifndef MAP_ANONYMOUS
        cfd = ::open("/dev/zero", O_RDONLY, 0);
#else
        cfd = -1;
        uc_mmap_attr |= MAP_ANONYMOUS;
#endif
#endif
        diskUpdateCount = (int*)mmap(NULL, nPages*sizeof(int),
                                     PROT_READ|PROT_WRITE, uc_mmap_attr, cfd, 0);
        if (diskUpdateCount == (int*)-1) {
            int status = errno;
            diskUpdateCount = NULL;
            if (cfd >= 0) {
                ::close(cfd);
            }
            return status;
        }
        int maxCount = 0;
        rootPage = dbMalloc(pageSize);
        for (int i = 0; i < nPages; i++) {
            int count = diskUpdateCount[i];
            currUpdateCount[i] = count;
            if (count > maxCount) {
                maxCount = count;
            }
        }
        updateCounter = maxCount;
        nRecovered = 0;
        recoveredEvent.open(true);
        syncEvent.open();
        startSync();
    }
#endif
#ifdef FUZZY_CHECKPOINT
    writer = new dbFileWriter(this);
#endif
    return ok;
}

bool dbFile::write(size_t pos, void const* ptr, size_t size)
{
    if ((size_t)lseek(fd, pos, SEEK_SET) != pos
        || (size_t)::write(fd, mmapAddr + pos, size) != size)
    {
        dbTrace("Failed to save page to the disk, position=%ld, size=%ld, error=%d\n",
                (long)pos, (long)size, errno);
        return false;
    }
    return true;
}

#if defined(REPLICATION_SUPPORT)
void dbFile::syncToDisk()
{
    syncThread.setPriority(dbThread::THR_PRI_LOW);
    dbCriticalSection cs(syncCS);
    while (doSync) {
        size_t i, j, k;
        int maxUpdated = 0;
        for (i = 0; i < mmapSize >> dbModMapBlockBits;) {
            int updateCounters[dbMaxSyncSegmentSize];
            for (j=i; j < (mmapSize >> dbModMapBlockBits) && j-i < dbMaxSyncSegmentSize
                     && currUpdateCount[j] > diskUpdateCount[j]; j++)
            {
                updateCounters[j-i] = currUpdateCount[j];
            }
            if (i != j) {
                size_t pos = (i << dbModMapBlockBits) & ~(pageSize-1);
                size_t size = (((j-i) << dbModMapBlockBits) + pageSize - 1) & ~(pageSize-1);
#ifdef NO_MMAP
                write(pos, mmapAddr + pos, size);
#else
                msync(mmapAddr + pos, size, MS_SYNC);
#endif
                for (k = 0; i < j; k++, i++) {
                    diskUpdateCount[i] = updateCounters[k];
                }
                maxUpdated = i;
            } else {
                i += 1;
            }
            if (!doSync) {
                return;
            }
        }
        if (maxUpdated != 0) {
            msync((char*)diskUpdateCount, maxUpdated*sizeof(int), MS_SYNC);
        }
        if (closing && maxUpdated == 0) {
            return;
        } else {
            syncEvent.wait(syncCS, dbSyncTimeout);
        }
    }
}
#endif


int dbFile::create(const char* name, bool)
{
    mmapAddr = NULL;
    fd = ::open(name, O_RDWR|O_TRUNC|O_CREAT, 0666);
    if (fd < 0) {
        return errno;
    }
    return ok;
}

int dbFile::read(void* buf, size_t& readBytes, size_t size)
{
    long rc = ::read(fd, buf, size);
    if (rc < 0) {
        readBytes = 0;
        return errno;
    }
    readBytes = rc;
    return ok;
}

int dbFile::write(void const* buf, size_t& writtenBytes, size_t size)
{
    long rc = ::write(fd, buf, size);
    if (rc < 0) {
        writtenBytes = 0;
        return errno;
    }
    writtenBytes = rc;
    return ok;
}

int dbFile::setSize(size_t size, char const*, bool)
{
#ifdef REPLICATION_SUPPORT
    dbCriticalSection cs1(syncCS);
    dbCriticalSection cs2(replCS);
#endif
#if defined(DISKLESS_CONFIGURATION) || defined(USE_SYSV_SHARED_MEMORY)
    assert(false);
#else
#if defined(NO_MMAP) || defined(REPLICATION_SUPPORT)
    int newPageMapSize = (size + dbModMapBlockSize*32 - 1) >> (dbModMapBlockBits + 5);
    int* newPageMap = new int[newPageMapSize];
    memcpy(newPageMap, pageMap, pageMapSize*sizeof(int));
    memset(newPageMap + pageMapSize, 0,
           (newPageMapSize-pageMapSize)*sizeof(int));
    delete[] pageMap;
    pageMapSize = newPageMapSize;
    pageMap = newPageMap;
#endif
#ifdef NO_MMAP
    char* newBuf = (char*)valloc(size);
    if (newBuf == NULL) {
        return errno;
    }
    memcpy(newBuf, mmapAddr, mmapSize);
    free(mmapAddr);
    mmapAddr = newBuf;
    mmapSize = size;
    if (ftruncate(fd, size) != ok) {
        return errno;
    }
#else
    if (munmap(mmapAddr, mmapSize) != ok ||
        (!readonly && ftruncate(fd, size) != ok) ||
        (mmapAddr = (char*)mmap(NULL, size, readonly ? PROT_READ : PROT_READ|PROT_WRITE,
                                MAP_SHARED, fd, 0)) == (char*)-1)
    {
        return errno;
    }
#endif
    mmapSize = size;
#endif
    return ok;
}

int dbFile::flush(bool physical)
{
#if defined(REPLICATION_SUPPORT)
    dbCriticalSection cs(replCS);
    if (db == NULL) {
        physical = true;
    }
    if (!physical) {
        updateCounter += 1;
    }
#endif
#if defined(REPLICATION_SUPPORT) || (defined(NO_MMAP) && !defined(DISKLESS_CONFIGURATION))
    int* map = pageMap;
    for (size_t i = 0, n = pageMapSize; i < n; i++) {
        if (map[i] != 0) {
            size_t pos = i << (dbModMapBlockBits + 5);
            unsigned mask = map[i];
            int count = 0;
            do {
                size_t size = 0;
                while ((mask & 1) == 0) {
                    pos += dbModMapBlockSize;
                    mask >>= 1;
                    count += 1;
                }
                while (true) {
                    do {
#ifdef REPLICATION_SUPPORT
                        if (!physical) {
                            currUpdateCount[(pos + size) >> dbModMapBlockBits] = updateCounter;
                        }
#endif
                        size += dbModMapBlockSize;
                        mask >>= 1;
                        count += 1;
                    } while ((mask & 1) != 0);
                    if (i+1 < n && count == 32 && size < dbMaxSyncSegmentSize*dbModMapBlockSize
                        && (map[i+1] & 1) != 0)
                    {
                        map[i] = 0;
                        mask = map[++i];
                        count = 0;
                    } else {
                        break;
                    }
                }
#if defined(REPLICATION_SUPPORT)
                if (db != NULL) {
                    if (!physical) {
                        for (int j = db->nServers; --j >= 0;) {
                            if (db->con[j].status == dbReplicatedDatabase::ST_STANDBY) {
                                ReplicationRequest rr;
                                rr.op = ReplicationRequest::RR_UPDATE_PAGE;
                                rr.nodeId = db->id;
                                rr.page.updateCount = updateCounter;
                                rr.page.offs = pos;
                                rr.size = size;
                                db->writeReq(j, rr, mmapAddr + pos, size);
                            }
                        }
                    }
                    pos += size;
                    continue;
                }
#else
#ifdef FUZZY_CHECKPOINT
                writer->put(pos, mmapAddr + pos, size);
#else
                if (!write(pos, mmapAddr + pos, size)) {
                    return errno;
                }
#endif
#endif
                pos += size;
            } while (mask != 0);
            map[i] = 0;
        }
    }
#endif
#if !defined(NO_MMAP) && !defined(DISKLESS_CONFIGURATION) && !defined(REPLICATION_SUPPORT) && !defined(NO_FLUSH_ON_COMMIT)
    if (msync(mmapAddr, mmapSize, MS_SYNC) != ok) {
        return errno;
    }
#endif
    return ok;
}

int dbFile::erase()
{
#ifdef USE_SYSV_SHARED_MEMORY
    shmem.erase();
#endif
    return ok;
}

int dbFile::close()
{
#if defined(REPLICATION_SUPPORT)
    if (db != NULL) {
        closing = true;
        stopSync();
        {
            dbCriticalSection cs(replCS);
            if (nRecovered != 0) {
                recoveredEvent.wait(replCS);
            }
        }
        syncEvent.close();
        recoveredEvent.close();
        munmap((char*)diskUpdateCount, getMaxPages()*sizeof(int));
        if (cfd >= 0) {
            ::close(cfd);
        }
    }
    delete[] currUpdateCount;
    currUpdateCount = NULL;
    dbFree(rootPage);
    rootPage = NULL;
#endif // REPLICATION_SUPPORT

    if (mmapAddr != NULL) {
#ifdef NO_MMAP
        int rc = flush();
        if (rc != ok) {
            return rc;
        }
#ifdef FUZZY_CHECKPOINT
        delete writer;
#endif
#endif
#ifdef USE_SYSV_SHARED_MEMORY
        shmem.close();
#elif defined(NO_MMAP)
        free(mmapAddr);
#else
        if (munmap(mmapAddr, mmapSize) != ok) {
            return errno;
        }
#endif
        mmapAddr = NULL;
#if defined(NO_MMAP) || defined(REPLICATION_SUPPORT)
        delete[] pageMap;
#endif
    }
    return fd >= 0 && ::close(fd) != ok ? errno : ok;
}

char* dbFile::errorText(int code, char* buf, size_t bufSize)
{
    strncpy(buf, strerror(code), bufSize-1);
    buf[bufSize-1] = '\0';
    return buf;
}

#endif

END_FASTDB_NAMESPACE
