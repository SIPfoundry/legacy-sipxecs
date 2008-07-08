// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "xmlparser/tinyxml.h"
#include "xmlparser/XmlErrorMsg.h"
#include "ProcessCmd.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// initializes by parsing a Command type element from sipXecs-process schema
ProcessCmd* ProcessCmd::parseCommandDefinition(const TiXmlElement* definition ///< any 'Command' type 
                                               )
{
   OsSysLog::add(FAC_WATCHDOG, PRI_NOTICE, "ProcessCmd::parseCommandDefinition "
                 "STUB - NOT IMPLEMENTED");
                 
   return new ProcessCmd("dummyuser","/dummy/path");
}

/// Execute the command.
void ProcessCmd::execute()
{
   OsSysLog::add(FAC_WATCHDOG, PRI_NOTICE, "ProcessCmd::execute "
                 "STUB - NOT IMPLEMENTED");
}

   
/// destructor
ProcessCmd::~ProcessCmd()
{
}

ProcessCmd::ProcessCmd(const char* user,
                       const char* workingDirectory
                       ) :
   mWorkingDirectory(workingDirectory),
   mUser(user)
{
}
