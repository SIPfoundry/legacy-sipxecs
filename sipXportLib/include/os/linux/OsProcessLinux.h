//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsProcessLinux_h_
#define _OsProcessLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsProcess.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

/**
 * Encapsulates a pid, and allows launching, querying, killing along with
 * other things that you can normally do with a process.
*/
class OsProcessLinux : public OsProcessBase
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///Default constructor
   OsProcessLinux();

/// Destructor
   virtual ~OsProcessLinux();

/* ============================ MANIPULATORS ============================== */
///   Launches, as a separate (child) process, an application as specified with the requested
///   priority.
    virtual OsStatus launch(
                            UtlString &rAppName, ///< Full application name to launch.
                            UtlString parameters[], /**< Parameters to be passed into the
                                                     *   application on startup. */
                            OsPath &startupDir,  /**< Directory path of where the
                                                  *   application is located. */
                            OsProcessPriorityClass prio = NormalPriorityClass,
                                                ///< Priority that the process is to run under.
                            UtlBoolean bExclusive = FALSE, /**< TRUE indicates process with
                                                            * same name must not already exist.*/
                            UtlBoolean bIgnoreChildSignals = TRUE  /**< TRUE indicates that child
                                                                    * signals will be ignored and
                                                                    * exit status cannot be
                                                                    * returned.  */
                            );

/**<
    \par
     <b>Note:</b> When the new child process exits, a signal SIGCHLD occurs
     (unless bIgnoreChildSignals is TRUE).  If
     the parent process is handling signals (not using the default signal handling),
     it must also handle the SIGCHLD signal (typically ignoring it).  The parent process
     <b>must</b> use the "wait" method to obtain the return code from the child process
     and to properly clean up the "zombie" (also known as defunct) process.  If the parent
     process fails to do a "wait", all child "zombie" processes will be cleaned up by the
     system when the parent exits.
     If bIgnoreChildSignals is TRUE (default), no SIGCHLD signal will occur,
     the child process will be completely cleaned up, and
     a return from "wait" will always be zero (i.e. success).
    \par
     Pipes are set up between the parent and the child to capture stdout and
     stderr.  The calling application should call captureOutput() in a loop
     until it returns non-zero, and handle the output message appropriately.

    @returns
       - TRUE if the child process was started okay.
       - FALSE otherwise.
*/



    /// Kills the process that was launched.
    virtual OsStatus kill();

    /// Changes the process priority.  Must own the process for this to be legal.
    virtual OsStatus setPriority(int prio ///< Priority to try and set for the process.
                                );

    /// Given a PID, this method will fill in the process passed in so the user
    /// can then manipulate it
    static OsStatus getByPID(PID pid, ///< Process Id to match against.
                             OsProcessLinux &rProcess /**< Process object returned that matches
                                                       *   the PID. */
                            );

       /// Sets the standard input, output and/or stderror
       virtual OsStatus setIORedirect(OsPath &rStdInputFilename,  /**< Path and filename to use
                                                                   *   for standard input. */
                                      OsPath &rStdOutputFilename, /**< Path and filename to use
                                                                   *   for standard output. */
                                      OsPath &rStdErrorFilename   /**< Path and filename to use for
                                                                   *   standard error. */
                                  );

/* ============================ ACCESSORS ================================= */

    /// Returns the current process id.
    static PID getCurrentPID();

    /// Returns the current thread id.
    static pthread_t getCurrentThreadId();

    /// Returns the process priority.  Must own the process for this to be legal.
    virtual OsStatus getPriority(int &rPrio ///< Priority of the running process.
                                );

    /// Returns the Priority Class for this process.  Priority is a function of the class.
    virtual OsStatus getPriorityClass(OsProcessPriorityClass &rPrioClass
                                      ///< Priority class of the running process.
                                     );

    /// Returns the min priority base on which class is selected
    virtual OsStatus getMinPriority(int &rMinPrio
                                    ///< Mininum priority base for the running process.
                                   );

    /// Returns the max priority base on which class is selected
    virtual OsStatus getMaxPriority(int &rMaxPrio
                                    ///< Maximum priority base for the running process.
                                   );

    /// Returns full information on process, including priority.
    /// See OsProcessInfo for more information
    virtual OsStatus getInfo(OsProcessInfo &rProcessInfo
                             ///< Process information for the running process.
                            );

     /// How long has this process been running for?
    virtual OsStatus getUpTime(OsTime &rUpTime
                               ///< The amount of time the process has been running.
                              );

/* ============================ INQUIRY =================================== */

    /// Returns TRUE if child process is still active.  FALSE if not.
    virtual UtlBoolean isRunning () const ;

    /// Waits n seconds for the child process to terminate.
    /// Passing in 0 seconds waits indefinetely.
    /// Returns the child process return code which may be an exit code
    /// signal code or stop status depending on how the process finished.
    int wait(int WaitInSecs  ///< Amount of seconds to wait.
            );

    /// Read messages from the child process on stdout or stderr.
    /// Returns the total number of bytes read.  The Msg strings are cleared on entry.
    /// When it returns 0, the child has exited and the parent should call wait(0)
    /// to remove zombies.
    /// If either parameter is NULL, the associated stream will not be read from
    /// (note however that this may cause buffers to fill and the process to hang).
    /// No attempt is made to read until newline.
    int getOutput(UtlString* stdoutMsg,  ///< message read from stdout
                  UtlString* stderrMsg   ///< message read from stderr
                  );
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */


#endif  // _OsProcessLinux_h_
