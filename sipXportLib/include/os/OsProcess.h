//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsProcess_h_
#define _OsProcess_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"
#include "os/OsConfigDb.h"
#include "os/OsFS.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
const short PID_STR_LEN = 20;  // long enough for 64 bit CPUs.
typedef pid_t PID;

typedef struct OS_PROCESS_STRUCT
{
    PID processID;
    //: Process Id of process specified by object

    PID parentProcessID;
    //: Parent Process Id of process specified by object

    UtlString name;
    //: Name of process.  Does not include full path, just executable name.

    UtlString commandline;
    //: Command line used to start the process

    int prioClass;
    //: Current priority of process


} *pOsProcessInfo, OsProcessInfo;


// FORWARD DECLARATIONS

//: This encapsulates a pid, and allows querying, killing and all the
//: other cool things you want to do to a process.

class OsProcessBase
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend class OsProcessIteratorWnt;
    friend class OsProcessIteratorVxw;
    friend class OsProcessIteratorLinux;
    friend class OsProcessIteratorHpux;
    friend class OsProcessIteratorFreeBSD;

    enum OsProcessPriorityClass {
        IdlePriorityClass = 0,
        NormalPriorityClass = 1,
        HighPriorityClass = 2,
        RealtimePriorityClass = 3
        };


      //!enumcode: IdlePriorityClass       - Lowest priority
      //!enumcode: NormalPriorityClass     - Default priority
      //!enumcode: HighPriorityClass       - High
      //!enumcode: RealtimePriorityClass   - Very High

/* ============================ CREATORS ================================== */
   OsProcessBase();
     //:Default constructor

/* ============================ MANIPULATORS ============================== */
    virtual OsStatus launch(UtlString &rAppName, UtlString rParameters[],OsPath &startDir,
                    OsProcessPriorityClass prio = NormalPriorityClass, UtlBoolean bExclusive = FALSE, UtlBoolean bIgnoreChildSignals = TRUE) = 0;
     //: Pass the appname and parameters to start the process.
     //:  the Appname could just be the exe name, in which case it is search using your PATH.
     //: startDir is the default directory the app will start from.
     //: Returns OS_SUCCESS if process started ok.
     //: prio specifies the priority class.  You may also get the range and manipulate
     //: the values yourself though setPriority
     //: If bExclusive is TRUE and another process by the same name already
     //: is running the return is OS_FAILED
     //: If bIgnoreChildSignals is FALSE, the parent process or the default handler will
     //: need to handle the signal SIGCHLD.


    virtual OsStatus kill() = 0;
     //: Kills the process specified by pid

    virtual OsStatus setPriority(int prio) = 0;
     //: Changes the process priority.  Must own the process for this to be legal.
     //: Returns TRUE if the priority was set correctly.
     //: Returns FALSE if the priority could not be set.
     //: Must be in the range specified by the getMin and Max Priorities.

    virtual OsStatus setEnv(UtlString &rKey, UtlString &rValue);
    //: The presets an environment variable. This does NOT set the OS env variable.
    //: It will set the OS variable just before launch.  Then clears it after the launch.
    //: The process object will continue to contain the env setting, the current OS, however does not.

    virtual OsStatus unsetEnv(UtlString &rKey);
    //: The removes the variable from the process object only.  It does not modify OS variables.


/* ============================ ACCESSORS ================================= */
    static OsStatus getByPID(PID pid, OsProcessBase &rProcess);
    //: Given a PID, this method will fill in the process passed in so the user
    //: can then manipulate it

    static PID getCurrentPID();
     //: returns the current process ID.
     // This Id is unique within the entire host, in that any two simultaneous
     // executions that do not share their memory space will have different
     // values from getCurrentPID().

    virtual PID getPID();
    //: Returns the process id contained by this object

    virtual PID getParentPID();
     //: Returns the parent PID for this process.

    /// returns the thread ID currently running
    static pthread_t getCurrentThreadId();

    virtual OsStatus getProcessName(UtlString& rProcessName);
     //: Returns the current process name (full path).
     //: Returns OS_SUCCESS if name retrieved ok.
     //: Returns OS_FAILED if process not assigned yet,

    virtual OsStatus getPriority(int &rPrio) = 0;
     //: Returns the process priority. (1-255)
     //: Adjusted based on Priority Class

    virtual OsStatus getMinPriority(int &rMinPrio) = 0;
    //:returns the value repesenting the minimum priority

    virtual OsStatus getMaxPriority(int &rMaxPrio) = 0;
    //:returns the value repesenting the maximum priority

    virtual OsStatus getPriorityClass(OsProcessPriorityClass &rPrioClass) = 0;
    //: Returns the Current PriorityClass specified at launch

    OsStatus getEnv(UtlString &rKey, UtlString &rValue);
    //: Returns the os environment variable

    virtual OsStatus getInfo(OsProcessInfo& rProcessInfo) = 0;
     //: Returns full information on process, including priority.
     //: See OsProcessInfo for more information

    virtual OsStatus getUpTime(OsTime &rUpTime) = 0;
     //: How long has this process been running for?

/* ============================ INQUIRY =================================== */

    virtual UtlBoolean isRunning () const = 0;
     //: Returns TRUE if process is still active

    virtual int wait(int WaitInSecs = -1);
    //: waits for a process to complete before returning
    //: or exits when WaitInSecs has completed

    /// Read some text from the child process on stdout or stderr.
    /// Blocks until some text is available, then returns what text
    /// can be read at the moment.
    /// Does not wait for a newline before returning text, nor does it
    /// segment output based on newlines.
    /// Returns the total number of bytes read.
    /// When it returns 0, the child has exited and the parent should call wait(0)
    /// to remove zombies.
    /// If either parameter is NULL, the associated stream will not be read from
    /// (note however that this may cause buffers to fill and the process to hang).
    virtual int getOutput(UtlString* stdoutMsg, ///< message read from stdout
                          UtlString* stderrMsg  ///< message read from stderr
         ) = 0;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    PID mPID;
     //: Process this object represents

    PID mParentPID;
     //: Parent PID of this process

    UtlString mProcessName;
     //:Used to store the processname this object represents
     //:This should also be the FULL path name
     //:This is what was passed on startup

    UtlString mProcessCmdLine;
     //:Used to store the full command line that this object represents

    OsProcessPriorityClass mPrioClass;
    //: Priority Class specified on process startup

    UtlBoolean mExeclusive;
    //: Startup flag which specified whether multiple instances of process
    //: can exist.

    UtlString mParameters;
    //: Parameters specified at startup

    UtlString mStdErrorFilename;
    //: Stores the std error output stream specified by the user.
    UtlString mStdInputFilename;
    //: Stores the std input stream specified by the user.
    UtlString mStdOutputFilename;
    //: Stores the std  output stream specified by the user.

    int m_fdout[2];   /// copy of stdout pipe after fork
    int m_fderr[2];   /// copy of stderr pipe after fork

    OsConfigDb mEnvList;
    //: Place to store the env variables before launching the process

    virtual OsStatus ApplyEnv();
    //: Called by object before launch to set any environment variables set by user.

   virtual ~OsProcessBase();
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */


// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsProcess)
// with the OS-dependent realization of that type (e.g., OsMutexWnt).
#if defined(_WIN32)
#  include "os/Wnt/OsProcessWnt.h"
   typedef class OsProcessWnt OsProcess;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsProcessVxw.h"
   typedef class OsProcessVxw OsProcess;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsProcessLinux.h"
   typedef class OsProcessLinux OsProcess;
#else
#  error Unsupported target platform.
#endif

//now add this header.
#include "os/OsProcessIterator.h"

#endif  // _OsProcess_h_
