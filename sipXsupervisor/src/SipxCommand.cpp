//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlSListIterator.h"
#include "utl/UtlTokenizer.h"
#include "xmlparser/XmlErrorMsg.h"
#include "xmlparser/ExtractContent.h"

#include "SipxProcessCmd.h"
#include "SipxCommand.h"

// DEFINES
// CONSTANTS

const UtlContainableType SipxCommand::TYPE = "SipxCommand";

const size_t MAX_STATUS_MSGS = 100;  /// maximum number of status msgs to save
const char* commandReturnTag = "return.code";

// TYPEDEFS
// STATICS INITIALIZATION
// FORWARD DECLARATIONS

/// constructor
SipxCommand::SipxCommand(const UtlString& name ) :
   UtlString(name),
   mLock(OsMutex::Q_FIFO),
   mCommand(NULL),
   mNumStdoutMsgs(0),
   mNumStderrMsgs(0)
{
};

/// Read an entire command definition and return a command if definition is valid.
/// NOTE this function is not fully supported at this time.  It is implemented for test purposes only
SipxCommand* SipxCommand::createFromDefinition(const OsPath& definitionFile)
{
   SipxCommand* command = NULL;

   UtlString errorMsg;

   TiXmlDocument commandDefinitionDoc(definitionFile);
   if (commandDefinitionDoc.LoadFile())
   {
      // definition is well formed xml, at least
      TiXmlElement* processDefElem;
      if ((processDefElem = commandDefinitionDoc.RootElement()))
      {
         const char* rootElementName = processDefElem->Value();
         const char* definitionNamespace = processDefElem->Attribute("xmlns");
         if ( rootElementName && definitionNamespace )
         {
            // step through the top level elements

            bool definitionValid = true;

            TiXmlElement* nameElement = NULL;
            UtlString name;
            TiXmlElement* commandElement = NULL;

            // Get the 'name' element
            nameElement = processDefElem->FirstChildElement();
            if (nameElement && (0 == strcmp("name",nameElement->Value())))
            {

               if ( ! textContent(name, nameElement)
                     || name.isNull()
               )
               {
                  definitionValid = false;
                  XmlErrorMsg(commandDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxCommand::createFromDefinition "
                        "'name' element content is invalid %s",
                        errorMsg.data()
                  );
               }
#if 0
               // eventually we may have a commandManager, similar to processManager

               else if ((command = commandManager->findCommand(name)))
               {
                  definitionValid = false;
                  XmlErrorMsg(commandDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxCommand::createFromDefinition "
                        "duplicate command name '%s'\n"
                        "  %s\n"
                        "  previously defined in '%s'",
                        name.data(), errorMsg.data(), command->mDefinitionFile.data()
                  );
                  command = NULL; // so that we don't return or delete it
               }
#endif
            }

            else
            {
               definitionValid = false;
               XmlErrorMsg(commandDefinitionDoc,errorMsg);
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxCommand::createFromDefinition "
                     "required 'name' element is missing %s",
                     errorMsg.data()
               );
            }

            // Get the 'command' element
            if ( definitionValid )
            {
               if ( (commandElement = nameElement->NextSiblingElement())
                     && (0 == strcmp("command",commandElement->Value())))
               {
                  // defer parsing commands until SipxProcess object is created below
               }
               else
               {
                  definitionValid = false;
                  XmlErrorMsg(commandDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxCommand::createFromDefinition "
                        "required 'command' element is missing %s",
                        errorMsg.data()
                  );
               }
            }

            /* All the required top level elements have been found, so create
             * the SipxCommand object and invoke the parsers for the components. */
            if (definitionValid)
            {
               if (! (command = SipxCommand::createFromDefinition(name,
                                 commandDefinitionDoc,
                                 commandElement)))
               {
                  definitionValid = false;
                  XmlErrorMsg(commandDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxCommand::createFromDefinition "
                        "'command' content is invalid %s",
                        errorMsg.data()
                  );
               }

               if (definitionValid)
               {
#if 0
                  // eventually
                  SipxCommandManager::getInstance()->save(command);
#endif
               }
            }
         }
         else
         {
            XmlErrorMsg(commandDefinitionDoc,errorMsg);
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxCommand::createFromDefinition "
                  "invalid root element '%s' in namespace '%s'\n"
                  //"should be '%s' in namespace '%s' "
                  "%s",
                  rootElementName, definitionNamespace,
                  //SipXecsProcessRootElement, SipXecsProcessNamespace,
                  errorMsg.data()
            );
         }
      }
      else
      {
         XmlErrorMsg(commandDefinitionDoc,errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxCommand::createFromDefinition "
               "root element not found in '%s': %s",
               definitionFile.data(), errorMsg.data()
         );
      }
   }
   else
   {
      UtlString errorMsg;
      XmlErrorMsg(commandDefinitionDoc,errorMsg);
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
            "SipxCommand::createFromDefinition failed to load '%s': %s",
            definitionFile.data(), errorMsg.data()
      );
   }

   return command;
}

/// Read a command definition fragment and return a command if definition is valid.
SipxCommand* SipxCommand::createFromDefinition( const UtlString& name,
                                                const TiXmlDocument& processDefinitionDoc,
                                                const TiXmlElement* commandElement // any 'Command'
      )
{
   SipxCommand* command = NULL;
   if ((command = new SipxCommand(name)))
   {
      bool definitionValid = true;
      UtlString errorMsg;

      if (! (command->mCommand = SipxProcessCmd::parseCommandDefinition(processDefinitionDoc,
                                                                        commandElement)))
      {
         definitionValid = false;
         XmlErrorMsg(processDefinitionDoc, errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxCommand::createFromDefinition "
            "'command' content is invalid %s", errorMsg.data() );
      }

      if (definitionValid)
      {
#if 0
         // eventually we may want to track Commands the same way we do Processes
         SipxCommandManager::getInstance()->save(command);
#endif
      }
      else
      {
         // something is wrong, so get rid of the invalid SipxCommand object
         delete command;
         command = NULL;
      }
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxCommand::createFromDefinition "
         "failed to create SipxCommand object for '%s'", name.data());
   }

   return command;
}

bool SipxCommand::isRunning()
{
   OsLock mutex(mLock);

   return mCommand->isRunning();
}

bool SipxCommand::execute()
{
   OsLock mutex(mLock);

   if ( mCommand->isRunning() )
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE,
                    "SipxCommand[%s]::execute failed; command is already running",
                    data());
      return false;
   }
   mCommand->execute(this);
   return true;
}


void SipxCommand::evCommandStarted(const SipxProcessCmd* command)
{
   clearCommandMessages();
}

void SipxCommand::evCommandStopped(const SipxProcessCmd* command, int rc)
{
   UtlString msg;
   msg.appendNumber(rc);

   OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "SipxCommand[%s] returned %d",
                 data(), rc);
   addCommandMessage(commandReturnTag, msg);
}

void SipxCommand::evCommandOutput(const SipxProcessCmd* command,
                                  OsSysLogPriority pri,
                                  UtlString output)
{
   // The output is sure to contain newlines, and may contain several lines.
   // Clean it up before dispatching.
   UtlTokenizer tokenizer(output);
   UtlString    msg;
   while ( tokenizer.next(msg, "\r\n") )
   {
      logCommandOutput(pri, msg);
   }
}

/// Determine whether or not the values in a containable are comparable.
UtlContainableType SipxCommand::getContainableType() const
{
   return TYPE;
}


/// Return any messages accumulated during the last run of the command
/// The caller is responsible for freeing the memory used for the strings.
void SipxCommand::getCommandMessages(UtlSList& statusMessages)
{
   OsLock mutex(mLock);

   statusMessages.removeAll();
   UtlSListIterator messages(mCommandMessages);
   UtlString* message = NULL;
   while ((message = dynamic_cast<UtlString*>(messages())))
   {
      statusMessages.append(new UtlString(*message));
   }
}

/// Clear any messages accumulated so far and reset log counters
void SipxCommand::clearCommandMessages()
{
   OsLock mutex(mLock);

   mCommandMessages.destroyAll();
   mNumStdoutMsgs = 0;
   mNumStderrMsgs = 0;
}

/// Save output from the command so it can be queried later
void SipxCommand::addCommandMessage(const char* msgTag, UtlString& msg)
{
   OsLock mutex(mLock);

   // only keep a limited amount of command output.
   if ( mCommandMessages.entries() > MAX_STATUS_MSGS )
   {
      delete mCommandMessages.at(0);
      mCommandMessages.removeAt(0);
   }

   char buf [1024];
   snprintf(buf, sizeof(buf), "%s: %s", msgTag, msg.data());
   mCommandMessages.append(new UtlString(buf));
}


/// Save and log a command output message
void SipxCommand::logCommandOutput(OsSysLogPriority pri, UtlString& msg)
{
   OsLock mutex(mLock);

   UtlString msgTag;
   if ( pri == PRI_ERR )
   {
      msgTag = stderrMsgTag;
      msgTag.appendNumber(++mNumStderrMsgs);
   }
   else
   {
      msgTag = stdoutMsgTag;
      msgTag.appendNumber(++mNumStdoutMsgs);
   }
   addCommandMessage(msgTag, msg);
   OsSysLog::add(FAC_SUPERVISOR, pri, "SipxCommand[%s]::commandOutput '%s'",
                 data(), msg.data());
}

/// destructor
SipxCommand::~SipxCommand()
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                 "~SipxCommand %s", data());

   {
      OsLock mutex(mLock);
      if (mCommand && mCommand->isRunning())
      {
         mCommand->kill();
      }
   }
   OsTask::delay(100);   // allow process to shutdown

   OsLock mutex(mLock);
   clearCommandMessages();
   if (mCommand)
   {
      delete mCommand;
      mCommand = NULL;
   }
};


