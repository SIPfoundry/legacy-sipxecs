//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsServerTaskWaitable_h_
#define _OsServerTaskWaitable_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsServerTask.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

/** Subclass of OsServerTask that uses the "pipe trick" so that the thread
 *  can wait for incoming messages with poll().
 */
/**
 *  The external interface of OsServerTaskWait is the same as that for
 *  OsServerTask, with the exception that OsServerTaskWait::getFd() returns
 *  a file descriptor which will be ready-to-read when a message is present
 *  in the message queue.
 */

class OsServerTaskWaitable : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsServerTaskWaitable(const UtlString& name = "",
                        void* pArg = NULL,
                        const int maxRequestQMsgs = DEF_MAX_MSGS,
                        const int priority = DEF_PRIO,
                        const int options = DEF_OPTIONS,
                        const int stackSize = DEF_STACKSIZE);

   virtual
   ~OsServerTaskWaitable();

/* ============================ MANIPULATORS ============================== */

   /// Posts a message to this task.
   virtual OsStatus postMessage(const OsMsg& rMsg,
                                const OsTime& rTimeout = OsTime::OS_INFINITY,
                                UtlBoolean sentFromISR = FALSE);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

   /** Return the file descriptor which is ready-to-read when a message is
    *  is in the queue.
    */
   int getFd(void) const;

   /** Return TRUE if creating the object has succeeded and it can be used.
    *  Return FALSE if creating the object has failed.  (That is, the pipe
    *  could not be opened.)
    */
   UtlBoolean isOk(void) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// The entry point for the task.
   virtual int run(void* pArg);
   /**<
    * This version has the same functionality as OsServerTask::run, but
    * is modified to read one byte from the pipe every time a message is
    * read from the queue.
    * In practice, the user of this class will want to write a run() method
    * with a more complex waiting logic, and should use this run() method
    * as a model.
    */

   /** The object controls a pipe which is used to make the message queue
    *  waitable.  Every time a message is added to the queue, a byte is
    *  written into the pipe, and every time a message is removed from the
    *  queue, a byte is removed from the pipe.  Thus, the pipe is readable
    *  when and only when there is one or more messages in the queue.
    */
   // The file descriptors for reading and writing the pipe.
   // These are set to -1 if opening the pipe failed.  The OsServerTaskWaitable
   // is not usable and should be destroyed.  isOk() returns FALSE in this
   // circumstance.
   int mPipeReadingFd;
   int mPipeWritingFd;

   /// The limit we allow for fd's returned by pipe().
   //  fdLimit allows us to enforce some headroom in fd allocation
   //  between the fd's assigned by SipClient's and getdtablesize().
   static const int sFdLimit;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Copy constructor (not implemented for this class)
   OsServerTaskWaitable(const OsServerTaskWaitable& rOsServerTaskWaitable);

   /// Assignment operator (not implemented for this task)
   OsServerTaskWaitable& operator=(const OsServerTaskWaitable& rhs);

};

#endif  // _OsServerTaskWaitable_h_
