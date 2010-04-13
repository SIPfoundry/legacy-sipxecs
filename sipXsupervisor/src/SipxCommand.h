//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef SIPXCOMMAND_H
#define SIPXCOMMAND_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlSList.h"
#include "xmlparser/tinyxml.h"
#include "SipxProcessCmd.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipxProcessCmd;


/// Manage a single sipXecs command process.
/**
 * A SipxCommand object represents a command which performs some discrete task for a
 * sipXecs service.  The current partial implementation supports only configtest commands
 * instantiated by the SipxProcessManager by calling
 * the SipxCommand::createFromDefinition method, passing a pointer to an xml
 * process definition doc and the configtest command element within it.  For test purposes,
 * and indicating the intended direction, command elements can also be parsed from
 * a command definition file similar to the process definition files.  Eventually
 * we may have a CommandManager which would allow commands to be loaded from
 * definition files and would run them and return their output via xmlrpc.
 *
 */
class SipxCommand : public UtlString, SipxProcessCmdOwner
{
  public:

// ================================================================
/** @name           Constructor
 *
 */
///@{

   /// Read an entire command definition and return a command if definition is valid.
   static SipxCommand* createFromDefinition(const OsPath& definitionFile);

   /// Read a command definition fragment and return a command if definition is valid.
   static SipxCommand* createFromDefinition( const UtlString& name,
                                             const TiXmlDocument& processDefinitionDoc,
                                             const TiXmlElement* commandElement // any 'Command'
                                             );

   /// Return true if the command process is currently running
   bool isRunning();

   /// Run the command as a separate task (does not interfere with normal process)
   /// Returns false if the command is already running
   bool execute();

   /// Return any messages accumulated during the last run of the command
   void getCommandMessages(UtlSList& statusMessages);
   ///< The caller is responsible for freeing the memory used for the strings.

   /// Clear any messages accumulated so far and reset log counters
   void clearCommandMessages();

  private:
     /// Save output from the command so it can be queried later
     void addCommandMessage(const char* msgTag, UtlString& msg);

     /// Save and log a command output message
     void logCommandOutput(OsSysLogPriority pri, UtlString& msg);


   ///@}
// ================================================================
/** @name           Events
 *
 */
///@{

  public:
   /// Notify the SipxCommand that a command has completed starting.
   void evCommandStarted(const SipxProcessCmd* command);

   /// Notify the SipxCommand that a command has stopped.
   void evCommandStopped(const SipxProcessCmd* command, int rc);

   /// Notify the SipxCommand that a command has received output.
   void evCommandOutput(const SipxProcessCmd* command,
                        OsSysLogPriority pri,
                        UtlString message);

  private:


// ================================================================
/** @name           Destructor
 *
 */
///@{

  public:

   /// destructor
   virtual ~SipxCommand();

///@}
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

// ================================================================

  private:

   OsMutex          mLock;          ///< must be held to access to other member variables.

   SipxProcessCmd*  mCommand;       ///< from the commands/command element

   UtlSList         mCommandMessages;  ///< list of messages from command
   int              mNumStdoutMsgs;    ///< number of messages received since last restart
   int              mNumStderrMsgs;    ///< number of messages received since last restart

   /// constructor
   SipxCommand(const UtlString& name/*,
               const UtlString& version,
               const OsPath&    definitionPath*/
               );

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipxCommand(const SipxCommand& nocopyconstructor);

   /// There is no assignment operator.
   SipxCommand& operator=(const SipxCommand& noassignmentoperator);
   // @endcond

};


#endif // SIPXCOMMAND_H
