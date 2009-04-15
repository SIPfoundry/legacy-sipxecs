// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>

#include <fcntl.h>
#include <kvm.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <limits.h>
#include <paths.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/freebsd/OsProcessIteratorFreeBSD.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsProcessIteratorFreeBSD::OsProcessIteratorFreeBSD() :
hProcessSnapshot(0)
{

    idx = 0;
}

// Destructor
OsProcessIteratorFreeBSD::~OsProcessIteratorFreeBSD()
{
}

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
OsStatus OsProcessIteratorFreeBSD::findFirst(OsProcess &rProcess)
{
    OsStatus retval;
    kvm_t *kd;
    struct kinfo_proc *kp;
    int nentries;
    char errbuf[_POSIX2_LINE_MAX];

    idx = 0;
    retval = OS_FAILED;
    kd = kvm_openfiles(_PATH_DEVNULL, _PATH_DEVNULL, NULL, O_RDONLY, errbuf);
    if (kd != NULL) {
        nentries = -1;
        kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &nentries);
        if (kp != NULL && nentries > idx) {
            rProcess.mProcessName = kp[idx].ki_comm;
            rProcess.mPID = kp[idx].ki_pid;
            rProcess.mParentPID = kp[idx].ki_ppid;
            retval = OS_SUCCESS;
        }
        kvm_close(kd);
    }

    return retval;
}

OsStatus OsProcessIteratorFreeBSD::findNext(OsProcess &rProcess)
{
    OsStatus retval;
    kvm_t *kd;
    struct kinfo_proc *kp;
    int nentries;
    char errbuf[_POSIX2_LINE_MAX];

    idx++;
    retval = OS_FAILED;
    kd = kvm_openfiles(_PATH_DEVNULL, _PATH_DEVNULL, NULL, O_RDONLY, errbuf);
    if (kd != NULL) {
        nentries = -1;
        kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &nentries);
        if (kp != NULL && nentries > idx) {
            rProcess.mProcessName = kp[idx].ki_comm;
            rProcess.mPID = kp[idx].ki_pid;
            rProcess.mParentPID = kp[idx].ki_ppid;
            retval = OS_SUCCESS;
        }
        kvm_close(kd);
    }

    return retval;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/*
 * We need to do this differently under FreeBSD.  We could open the
 * appropriate /proc/nnnn/status file as shown above, but OsFile
 * wants to call fcntl() to lock the file when it is opened.  FreeBSD
 * does not allow locking the procfs file and fails the fcntl() call.
 * This in turn means we don't get to read the PID information and
 * all sorts of bad things start happening.
 *
 * Rather than try to find a way to bypass the fcntl() call in some
 * cases, let's just go ahead and use another method to get this PID
 * information.
 */
OsStatus OsProcessIteratorFreeBSD::readProcFile(OsPath &procDirname, OsProcess & rProcess)
{
    OsStatus retval;
    kvm_t *kd;
    struct kinfo_proc *kp;
    int nentries, pid;
    char errbuf[_POSIX2_LINE_MAX];

    retval = OS_FAILED;
    kd = kvm_openfiles(_PATH_DEVNULL, _PATH_DEVNULL, NULL, O_RDONLY, errbuf);
    if (kd != NULL) {
        nentries = -1;
        pid = atoi(procDirname.data());
        kp = kvm_getprocs(kd, KERN_PROC_PID, pid, &nentries);
        if (kp != NULL && nentries > 0) {
            rProcess.mProcessName = kp->ki_comm;
            rProcess.mPID = kp->ki_pid;
            rProcess.mParentPID = kp->ki_ppid;
            retval = OS_SUCCESS;
        }
        kvm_close(kd);
    }

    return retval;
}

/* ============================ FUNCTIONS ================================= */
