//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "xmlparser/tinyxml.h"
#include "xmlparser/XmlErrorMsg.h"
#include "xmlparser/ExtractContent.h"

#include "ProcessManager.h"
#include "Process.h"

#include "ProcessResourceManager.h"
#include "ProcessResource.h"

// DEFINES
// CONSTANTS
const UtlContainableType ProcessResource::TYPE = "ProcessResource";

const char* ProcessResource::ProcessResourceTypeName = "process";

// TYPEDEFS
// FORWARD DECLARATIONS

// Factory method that parses a 'process' resource description element.
bool ProcessResource::parse(const TiXmlDocument& processDefinitionDoc, ///< process definition document
                            TiXmlElement* resourceElement, // 'process' element
                            Process* currentProcess        // whose resources are being read.
                            )
{
   /*
    * This is called by SipxResource::parse with any 'process' child of
    * the 'resources' element in a process definition.
    *
    * @returns NULL if the element was in any way invalid.
    */
   UtlString errorMsg;
   bool resourceIsValid;

   UtlString processName;
   resourceIsValid = textContent(processName, resourceElement);
   if (resourceIsValid)
   {
      if (!processName.isNull())
      {

         ProcessResourceManager* processResourceMgr = ProcessResourceManager::getInstance();

         ProcessResource* processResource;
         if (!(processResource = processResourceMgr->find(processName)))
         {
            processResource = new ProcessResource(processName, NULL);
         }

         for ( const TiXmlAttribute* attribute = resourceElement->FirstAttribute();
               resourceIsValid && attribute;
               attribute = attribute->Next()
              )
         {
            if (!(resourceIsValid =
                  processResource->SipxResource::parseAttribute(processDefinitionDoc,
                                                                attribute, currentProcess)
                  ))
            {
               OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "ProcessResource::parse "
                             "invalid attribute '%s'",
                             attribute->Name());
            }
         }

         if ( processResource->mFirstDefinition ) // existing resources are in the manager already
         {
            if (resourceIsValid)
            {
               processResource->mFirstDefinition = false;
               processResourceMgr->save(processResource);
            }
            else
            {
               delete processResource;
            }
         }
      }
      else
      {
         resourceIsValid = false;
         XmlErrorMsg(processDefinitionDoc, errorMsg);
         OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "ProcessResource::parse "
                       "process element is empty %s",
                       errorMsg.data());
      }
   }
   else
   {
      XmlErrorMsg(processDefinitionDoc, errorMsg);
      OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "ProcessResource::parse "
                    "invalid content in process element %s",
                    errorMsg.data());
   }

   return resourceIsValid;
}


// get a description of the ProcessResource (for use in logging)
void ProcessResource::appendDescription(UtlString&  description /**< returned description */) const
{
   description.append("process '");
   description.append(data());
   description.append("'");
}

/// A ProcessResource may not written by configuration update methods.
bool ProcessResource::isWriteable()
{
   return false;
}

/// If possible, get the corresponding Process object.
Process* ProcessResource::getProcess()
{
   // return the Process that has the same name as this ProcessResource. 
   return ProcessManager::getInstance()->findProcess(*this); 
}
   

// Whether or not the ProcessResource is ready for use by a Process.
bool ProcessResource::isReadyToStart()
{
   return false; // @TODO
}

// Whether or not it is safe to stop a Process using the ProcessResource.
bool ProcessResource::isSafeToStop()
{
   return ! isReadyToStart();
}

// Determine whether or not the values in a containable are comparable.
UtlContainableType ProcessResource::getContainableType() const
{
   return TYPE;
}

/// constructor
ProcessResource::ProcessResource(const char* uniqueId, Process* currentProcess) :
   SipxResource(uniqueId, currentProcess)
{
   mWritable=false;
}

/// destructor
ProcessResource::~ProcessResource()
{
}
