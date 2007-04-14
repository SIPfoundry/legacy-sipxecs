// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
#ifndef _processXMLCommon_
#define _processXMLCommon_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <signal.h>


// APPLICATION INCLUDES
#include "xmlparser/tinyxml.h"
#include "os/OsNameDb.h"
#include "os/OsDefs.h"
#include "os/OsProcess.h"
#include "os/OsProcessMgr.h"
#include "os/OsProcessIterator.h"
#include "processcgi/DependencyList.h"

// DEFINES
#define ACTION_START    "start"
#define ACTION_STOP     "stop"
#define ACTION_RESTART  "restart"
#define ACTION_STATUS   "status"
#define ACTION_VERIFY   "verify"
#define PROCESS_DIR     "process.d"

typedef OsStatus (*ProcessSubDoc)(TiXmlDocument &rootdoc, TiXmlDocument &subdoc);
     
OsStatus initProcessXMLLayer(UtlString &rProcessXMLPath, TiXmlDocument &rProcessXMLDoc, UtlString &rStrErrorMsg);
OsStatus startstopProcessTree(TiXmlDocument &rProcessXMLDoc, UtlString &rProcessAlias, UtlString &rActionVerb);
OsStatus WriteProcessXML(TiXmlDocument &doc, UtlString &buffer);
OsStatus VerifyProcess(UtlString &rAlias);
OsStatus findSubDocs(OsPath &path, TiXmlDocument &rootDoc, ProcessSubDoc addSubDoc);
OsStatus addWatchDogSubDoc(TiXmlDocument &rWatchDogXMLDoc, TiXmlDocument &subdoc);

OsStatus addProcessDefSubDoc(TiXmlDocument &rProcessXMLDoc, TiXmlDocument &subdoc);

#endif //_processXMLCommon__
