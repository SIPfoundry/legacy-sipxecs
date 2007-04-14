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


dbFile::dbFile()
{
    sharedName = NULL;
    mmapAddr = NULL;
    mmapSize = 0;

#ifdef REPLICATION_SUPPORT
    currUpdateCount = NULL; 
    diskUpdateCount = NULL;
    rootPage = NULL;
    db = NULL;
#endif
}

#if defined(REPLICATION_SUPPORT) || defined(NO_MMAP)
const int dbMaxSyncSegmentSize = 128*1024 / dbModMapBlockSize; 
#endif

#ifdef REPLICATION_SUPPORT

#include "database.h"

int dbFile::dbSyncTimeout = 1000; // one second

bool dbFile::updatePages(int nodeId, size_t pos, int pageUpdateCounter, int size)
{
    if (pos + size > mmapSize) { 
	size_t newSize = pos + size > mmapSize*2 ? pos + size : mmapSize*2;
	setSize(newSize, sharedName);
    }
    if (db->con[nodeId].respSock->read(mmapAddr + pos, size)) { 	
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

bool dbFile::concurrentUpdatePages(int nodeId, size_t pos, int pageUpdateCounter, int size)
{
    if (pos + size > mmapSize) { 
	size_t newSize = pos + size > mmapSize*2 ? pos + size : mmapSize*2;
        db->beginTransaction(dbDatabase::dbCommitLock);
	setSize(newSize, sharedName);
        ((dbHeader*)mmapAddr)->size = newSize;
        db->version = db->monitor->version += 1;
    }
    if (pos == 0 && size <= pageSize) { 
        if (!db->con[nodeId].respSock->read(rootPage, size)) { 	
            return false;
        }        
        if (((dbHeader*)rootPage)->curr != ((dbHeader*)mmapAddr)->curr) { 
            db->beginTransaction(dbDatabase::dbCommitLock);
            memcpy(mmapAddr, rootPage, size);
            // now readers will see updated data
            db->monitor->curr ^= 1;
            db->endTransaction();
        } else { 
            memcpy(mmapAddr, rootPage, size);
        }
    } else { 
        if (!db->con[nodeId].respSock->read(mmapAddr + pos, size)) { 	
            return false;
        }
    }
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

void dbFile::doRecovery(int nodeId, int* updateCounters, int nPages)
{
    ReplicationRequest rr;
    memset(updateCounters+nPages, 0, (getMaxPages() - nPages)*sizeof(int));
    int i, j, n; 

    if (db->con[nodeId].reqSock == NULL) { 
	char buf[256];
	socket_t* s = socket_t::connect(db->serverURL[nodeId], 
					socket_t::sock_global_domain, 
					dbReplicatedDatabase::dbRecoveryConnectionAttempts);
	if (!s->is_ok()) { 
	    s->get_error_text(buf, sizeof buf);
	    dbTrace("Failed to establish connection with node %d: %s\n",
		    nodeId, buf);
	    delete s;
	    return;
	} 
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

    while (true) { 
	int maxUpdateCount = 0;
	{ 
	    dbCriticalSection cs(syncCS);
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
		    rr.op = ReplicationRequest::RR_UPDATE_PAGE;
		    rr.nodeId = nodeId;
		    rr.size = (i-j)*dbModMapBlockSize;
		    rr.page.offs = (size_t)j << dbModMapBlockBits;
		    rr.page.updateCount = currUpdateCount[j];
		    if (!db->writeReq(nodeId, rr, mmapAddr + rr.page.offs, rr.size)) {
			delete[] updateCounters;
			return;
		    }
		    j = i;
		}
		if (currUpdateCount[i] > updateCounters[i]) { 
		    if (currUpdateCount[i] > maxUpdateCount) { 
			maxUpdateCount = currUpdateCount[i];
		    }
		    updateCounters[i] = currUpdateCount[i];
		} else { 
		    j = i + 1;
		}
	    }	   
	    if (i != j) { 
		rr.op = ReplicationRequest::RR_UPDATE_PAGE;
		rr.nodeId = nodeId;
		rr.size = (i-j)*dbModMapBlockSize;
		rr.page.offs = (size_t)j << dbModMapBlockBits;
		rr.page.updateCount = currUpdateCount[j];
		if (!db->writeReq(nodeId, rr, mmapAddr + rr.page.offs, rr.size)) {
		    delete[] updateCounters;
		    return;
		}
	    }	
	}
	{ 
	    dbCriticalSection cs(replCS);
	    if (maxUpdateCount == updateCounter) { 
		dbTrace("Complete recovery of node %d\n", nodeId);
		delete[] updateCounters;
		rr.op = ReplicationRequest::RR_STATUS;
		rr.nodeId = nodeId;
		db->con[nodeId].status = rr.status = dbReplicatedDatabase::ST_STANDBY;
		for (i = 0, n = db->nServers; i < n; i++) { 		    
		    if (db->con[i].status != dbReplicatedDatabase::ST_OFFLINE && i != db->id) {
			db->writeReq(i, rr); 
		    }
		}
		return;
	    }
	}
    }
}


#endif

bool dbFile::write(void const* buf, size_t size)
{
    size_t writtenBytes;
    bool result = write(buf, writtenBytes, size) == ok && writtenBytes == size;
    assert(result);
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

int dbFile::open(char const* fileName, char const* sharedName, bool readonly,
		 size_t initSize, bool replicationSupport)
{
    int status;
    size_t fileSize;
#ifndef DISKLESS_CONFIGURATION
    fh = CreateFile(fileName, readonly ? GENERIC_READ : (GENERIC_READ|GENERIC_WRITE), 
		    FILE_SHARE_READ | FILE_SHARE_WRITE, FASTDB_SECURITY_ATTRIBUTES, 
		    readonly ? OPEN_EXISTING : OPEN_ALWAYS,
		    FILE_FLAG_RANDOM_ACCESS
#ifdef NO_MMAP
		    |FILE_FLAG_NO_BUFFERING
#endif
#if 0 // not needed as we do explicit flush ???
		    |FILE_FLAG_WRITE_THROUGH
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
                           0, mmapSize, sharedName);
    status = GetLastError();
    if (mh == NULL) { 
	if (fh != INVALID_HANDLE_VALUE) { 
	    CloseHandle(fh);
	}
	return status;
    }
    mmapAddr = (char*)MapViewOfFile(mh, readonly 
				    ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, 
				    0, 0, 0);
    if (mmapAddr == NULL) { 
	status = GetLastError();
	CloseHandle(mh);
	if (fh != INVALID_HANDLE_VALUE) { 
	    CloseHandle(fh);
	}
	return status;
    } 
    if (status != ERROR_ALREADY_EXISTS && mmapSize > fileSize)
	//	&& osinfo.dwPlatformId != VER_PLATFORM_WIN32_NT) 
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
        diskUpdateCount = (int*)MapViewOfFile(cmh, FILE_MAP_ALL_ACCESS, 
                                              0, 0, 0);
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
    return ok; 
}

#if defined(REPLICATION_SUPPORT)
void dbFile::syncToDisk()
{
    syncThread.setPriority(dbThread::THR_PRI_LOW);
    dbCriticalSection cs(syncCS);
    while (doSync) { 
	int i, j, k; 
	int maxUpdated = 0;
	for (i = 0; i < int(mmapSize >> dbModMapBlockBits);) { 
	    int updateCounters[dbMaxSyncSegmentSize];
	    for (j=i; j < int(mmapSize >> dbModMapBlockBits) && j-i < dbMaxSyncSegmentSize 
		     && currUpdateCount[j] > diskUpdateCount[j]; j++)
	    {
		updateCounters[j-i] = currUpdateCount[j];
	    }
	    if (i != j) { 
		size_t pos = (i << dbModMapBlockBits) & ~(pageSize-1);
		size_t size = (((j-i) << dbModMapBlockBits) + pageSize - 1) & ~(pageSize-1);
#ifdef NO_MMAP
		DWORD written;
		if (SetFilePointer(fh, pos, NULL, FILE_BEGIN) != pos ||
		    !WriteFile(fh, mmapAddr + pos, size, &written, NULL) 
		    || written != (DWORD)size) 
		{ 
		    dbTrace("Failed to save page to the disk, position=%ld, size=%ld, error=%d\n",
			    (long)pos, (long)size, GetLastError());
		}
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
    fh = CreateFile(name, GENERIC_READ|GENERIC_WRITE, 0, FASTDB_SECURITY_ATTRIBUTES, CREATE_ALWAYS, 
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
    for (int i = 0, n = pageMapSize; i < n; i++) { 
	if (map[i] != 0) { 
	    size_t pos = (size_t)i << (dbModMapBlockBits + 5);
	    unsigned mask = map[i];
	    int count = 0;
	    do { 
		int size = 0;
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
                DWORD written;
                if (SetFilePointer(fh, pos, NULL, FILE_BEGIN) != pos ||
                    !WriteFile(fh, mmapAddr + pos, size, &written, NULL) 
                    || written != (DWORD)size) 
                { 
                    return GetLastError();
                }
#endif
		pos += size;
	    } while (mask != 0);
	    map[i] = 0;
	}
    }
#endif
#if !defined(NO_MMAP) && !defined(DISKLESS_CONFIGURATION) && !defined(REPLICATION_SUPPORT)
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
    mh = CreateFileMapping(fh, FASTDB_SECURITY_ATTRIBUTES, PAGE_READWRITE, 0, size, sharedName);
    int status = GetLastError();
    if (mh == NULL) { 
        printf("CreateFileMapping failed: %d\n", status);
	return status;
    }
    mmapAddr = (char*)MapViewOfFile(mh, FILE_MAP_ALL_ACCESS, 0, 0, 0);
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
    int len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			    NULL,
			    code,
			    0,
			    buf,
			    bufSize,
			    NULL);
    if (len == 0) { 
	char errcode[64];
	sprintf(errcode, "unknown error code %u", code);
	strncpy(buf, errcode, bufSize);
    }
#else
    char errcode[64];
    sprintf(errcode, "unknown error code %u", code);
    strncpy(buf, errcode, bufSize);
#endif
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
#define O_SYNC	O_FSYNC
#endif

#ifndef O_DSYNC
#define O_DSYNC O_SYNC
#endif


int dbFile::open(char const* name, char const*, bool readonly, size_t initSize, bool replicationSupport)
{
#ifdef USE_SYSV_SHARED_MEMORY
    if (!shmem.open(name, initSize)) { 
	return errno;
    }
    mmapSize = initSize;	   
    mmapAddr = shmem.get_base();
    fd = -1;
#else
    int mmap_attr = MAP_SHARED;
    int status;
#ifdef DISKLESS_CONFIGURATION
#ifndef MAP_ANONYMOUS 
    fd = ::open("/dev/zero", O_RDWR, 0);
#else
    fd = -1; 
    mmap_attr |= MAP_ANONYMOUS;
#endif
    mmapSize = initSize;
#else
    fd = ::open(name, readonly ? O_RDONLY : O_RDWR/*|O_DSYNC*/|O_CREAT, 0666);
    if (fd < 0) { 
	return errno;
    }
    mmapSize = lseek(fd, 0, SEEK_END); 
#endif
    if (!readonly && mmapSize < initSize) { 
	mmapSize = initSize;
	if (ftruncate(fd, mmapSize) != ok) {
	    status = errno;
	    if (fd >= 0) { 
		::close(fd);
	    }
	    return status;
	}
    }
#ifdef NO_MMAP
    size_t fileSize = mmapSize;
    if (!readonly && mmapSize < initSize) { 
	mmapSize = initSize;
    }
    mmapAddr = (char*)valloc(mmapSize);
    if (mmapAddr == NULL) { 
	status = errno;
	if (fd >= 0) { 
	    ::close(fd);
	}
	return status;
    }
    lseek(fd, 0, SEEK_SET); 
    if ((size_t)::read(fd, mmapAddr, fileSize) != fileSize) { 
	free(mmapAddr);
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
#endif // USE_SYSV_SHARED_MEMORY
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
        mmap_attr = MAP_SHARED;
#ifndef MAP_ANONYMOUS 
        cfd = ::open("/dev/zero", O_RDONLY, 0);
#else
        cfd = -1; 
        mmap_attr |= MAP_ANONYMOUS;
#endif    
#endif
        diskUpdateCount = (int*)mmap(NULL, nPages*sizeof(int), 
                                     PROT_READ|PROT_WRITE, mmap_attr, cfd, 0);
        if (diskUpdateCount == (int*)-1) { 
            status = errno;
            diskUpdateCount = NULL;
            if (cfd >= 0) { 
                ::close(cfd);
            }
            return status;
        }
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
    return ok;
}


#if defined(REPLICATION_SUPPORT)
void dbFile::syncToDisk()
{
    syncThread.setPriority(dbThread::THR_PRI_LOW);
    dbCriticalSection cs(syncCS);
    while (doSync) { 
	int i, j, k; 
	int maxUpdated = 0;
	for (i = 0; i < int(mmapSize >> dbModMapBlockBits);) { 
	    int updateCounters[dbMaxSyncSegmentSize];
	    for (j=i; j < (int)(mmapSize >> dbModMapBlockBits) && j-i < dbMaxSyncSegmentSize 
		     && currUpdateCount[j] > diskUpdateCount[j]; j++)
	    {
		updateCounters[j-i] = currUpdateCount[j];
	    }
	    if (i != j) {
		size_t pos = (i << dbModMapBlockBits) & ~(pageSize-1);
		size_t size = (((j-i) << dbModMapBlockBits) + pageSize - 1) & ~(pageSize-1);
#ifdef NO_MMAP
		if (lseek(fd, pos, SEEK_SET) != pos
		    || ::write(fd, mmapAddr + pos, size) != size) 
		{ 
		    dbTrace("Failed to save page to the disk, position=%ld, size=%ld, error=%d\n",
			    (long)pos, (long)size, errno);
		}
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
	    msync(diskUpdateCount, maxUpdated*sizeof(int), MS_SYNC);
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
#ifdef DISKLESS_CONFIGURATION
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
	ftruncate(fd, size) != ok ||
	(mmapAddr = (char*)mmap(NULL, size, PROT_READ|PROT_WRITE,
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
    for (int i = 0, n = pageMapSize; i < n; i++) { 
	if (map[i] != 0) { 
	    size_t pos = (size_t)i << (dbModMapBlockBits + 5);
	    unsigned mask = map[i];
	    int count = 0;
	    do { 
		int size = 0;
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
		if (lseek(fd, pos, SEEK_SET) != pos
		    || ::write(fd, mmapAddr + pos, size) != size) 
		{ 
		    return errno;
		}
#endif
		pos += size;
	    } while (mask != 0);
	    map[i] = 0;
	}
    }
#endif
#if !defined(NO_MMAP) && !defined(DISKLESS_CONFIGURATION) && !defined(REPLICATION_SUPPORT)
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
	munmap(diskUpdateCount, getMaxPages()*sizeof(int));
	if (cfd >= 0) { 
	    ::close(cfd);
	}
    }
    delete[] currUpdateCount;
    currUpdateCount = NULL;
    dbFree(rootPage);
    rootPage = NULL;
#endif
    if (mmapAddr != NULL) { 
#ifdef USE_SYSV_SHARED_MEMORY
	shmem.close();
#else
#ifdef NO_MMAP
	int rc = flush();
	if (rc != ok) { 
	    return rc;
	}
	free(mmapAddr);    
#else
	if (munmap(mmapAddr, mmapSize) != ok) { 
	    return errno;
	}
#endif
#endif
	mmapAddr = NULL;
#if defined(NO_MMAP) || defined(REPLICATION_SUPPORT)
        delete[] pageMap;
#endif
    }
    return fd < 0 && ::close(fd) != ok ? errno : ok;
}

char* dbFile::errorText(int code, char* buf, size_t bufSize)
{
    return strncpy(buf, strerror(code), bufSize);
}

#endif

