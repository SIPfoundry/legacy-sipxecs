//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIPXPROCESSCMD_H_
#define _SIPXPROCESSCMD_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsServerTask.h"
#include "os/OsProcess.h"
#include "utl/UtlString.h"
#include "utl/UtlSList.h"

// DEFINES
// The following tags are used by sipXconfig to display status messages nicely
// and should not be changed without consulting sipXconfig designers.
#define stdoutMsgTag "stdout.msg-"
#define stderrMsgTag "stderr.msg-"

// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlElement;
class SipxProcessCmdOwner;

/// Represents a command as defined by the Command type in sipXecs-process.xsd
/**
 * The process definition parser calls the parseCommandDefinition method of this class
 * to interpret each command element for the process, which returns NULL if the element
 * is not valid.
 *
 * To execute the command, the execute method is called (it does not return any status).
 */
class SipxProcessCmd : public UtlString, OsServerTask
{
  public:

   /// initializes by parsing a Command type element from sipXecs-process schema
   static SipxProcessCmd* parseCommandDefinition(const TiXmlDocument& processDefinitionDoc,
                                             ///< the process definition document
                                             const TiXmlElement* commandElement /**< any 'Command'
                                                                                 *   type element
                                                                                 */
                                             );
   ///< @returns NULL if the element contents are invalid.

   /// Execute the command and return appropriate event to the owner.
   void execute(SipxProcessCmdOwner* owner);

   /// Return true if the command process is currently running
   bool isRunning();

   /// Kill the command process
   void kill();

   /// destructor
   virtual ~SipxProcessCmd();

// ================================================================
/** @name           Container Support Operations
 *
 */
///@{

   /// Determine whether or not the values in a containable are comparable.
   virtual UtlContainableType getContainableType() const;
   /**<
    * This returns a unique type for UtlString
    */

   static const UtlContainableType TYPE;    ///< Class type used for runtime checking

///@}
  protected:

   SipxProcessCmd(const UtlString& execute,
              const UtlString& workingDirectory,
              const UtlString& user
              );
  private:

   OsPath      mWorkingDirectory; ///< Directory from which all process commands are exec'd
   UtlString   mUser;             ///< User that commands must be run as.
   UtlString   mExecutable;       ///< Fully qualified path to the command to be executed.
   UtlSList    mParameters;       ///< UtlStrings to be passed as arguments
   OsProcess   mProcess;          ///< Pointer to the process object

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipxProcessCmd(const SipxProcessCmd& nocopyconstructor);

   /// There is no assignment operator.
   SipxProcessCmd& operator=(const SipxProcessCmd& noassignmentoperator);
   // @endcond

   /// Process asynchronous request from application code
   virtual UtlBoolean handleMessage(OsMsg& rMsg);

   /// Handle the execute request in the separate command task
   void executeInTask(SipxProcessCmdOwner* owner);

   friend class SipxProcessDefinitionParserTest;
};


/** Class to implement an interface by which results can be returned from the SipxProcessCmd
 *  to its owner
 */
class SipxProcessCmdOwner
{
   public:
      virtual ~SipxProcessCmdOwner() {} // nominal destructor

      /// Notify the owner that a command has completed starting.
      virtual void evCommandStarted(const SipxProcessCmd* command) = 0;

      /// Notify the owner that a command has stopped.
      virtual void evCommandStopped(const SipxProcessCmd* command, int rc) = 0;

      /// Notify the owner that a command has received output.
      virtual void evCommandOutput(const SipxProcessCmd* command,
                           OsSysLogPriority pri,
                           UtlString message) = 0;

};


/**
 * Message sent to the SipxProcessCmd task to prevent application code from blocking
 * while task executes.
 */
class ExecuteMsg : public OsMsg
{
public:

   enum EventSubType
   {
      EXECUTE    = 1
   };

   /// Constructor.
   ExecuteMsg(//EventSubType eventSubType,
                  SipxProcessCmdOwner* owner            ///< owner
                  );

   /// Destructor
   virtual ~ExecuteMsg();

   // Component accessors.
   SipxProcessCmdOwner* getOwner( void ) const    {return mOwner;}

protected:
   static const UtlContainableType TYPE;   ///< Class type used for runtime checking

private:
   SipxProcessCmdOwner* mOwner;               ///< owner

   /// Copy constructor
   ExecuteMsg( const ExecuteMsg& rhs);
   virtual OsMsg* createCopy(void) const;
};

#endif // _SIPXPROCESSCMD_H_
