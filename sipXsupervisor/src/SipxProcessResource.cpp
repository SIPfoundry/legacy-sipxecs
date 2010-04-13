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

#include "SipxProcessManager.h"
#include "SipxProcess.h"

#include "SipxProcessResourceManager.h"
#include "SipxProcessResource.h"

// DEFINES
// CONSTANTS
const UtlContainableType SipxProcessResource::TYPE = "SipxProcessResource";

const char* SipxProcessResource::SipxProcessResourceTypeName = "process";

// TYPEDEFS
// FORWARD DECLARATIONS

// Factory method that parses a 'process' resource description element.
bool SipxProcessResource::parse(const TiXmlDocument& processDefinitionDoc, ///< process definition document
                            TiXmlElement* resourceElement, // 'process' element
                            SipxProcess* currentProcess        // whose resources are being read.
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

         SipxProcessResourceManager* processResourceMgr = SipxProcessResourceManager::getInstance();

         SipxProcessResource* processResource;
         if (!(processResource = processResourceMgr->find(processName)))
         {
            processResource = new SipxProcessResource(processName);
         }
         // do not register this resource as used by itself
         if( processResource->compareTo( currentProcess ) != 0 )
         {
            processResource->usedBy( currentProcess );
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
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcessResource::parse "
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
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcessResource::parse "
                       "process element is empty %s",
                       errorMsg.data());
      }
   }
   else
   {
      XmlErrorMsg(processDefinitionDoc, errorMsg);
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcessResource::parse "
                    "invalid content in process element %s",
                    errorMsg.data());
   }

   return resourceIsValid;
}


// get a description of the SipxProcessResource (for use in logging)
void SipxProcessResource::appendDescription(UtlString&  description /**< returned description */) const
{
   description.append("process '");
   description.append(data());
   description.append("'");
}

/// A SipxProcessResource may not written by configuration update methods.
bool SipxProcessResource::isWriteable()
{
   return false;
}

/// If possible, get the corresponding SipxProcess object.
SipxProcess* SipxProcessResource::getProcess()
{
   // return the SipxProcess that has the same name as this SipxProcessResource.
   return SipxProcessManager::getInstance()->findProcess(*this);
}


// Whether or not the SipxProcessResource is ready for use by a SipxProcess.
bool SipxProcessResource::isReadyToStart(UtlString& missingResource)
{
   SipxProcess* myProcess = getProcess();

   bool bReady = (myProcess && myProcess->isRunning());
   if ( !bReady )
   {
      missingResource = "";
      appendDescription(missingResource);
       OsSysLog::add(FAC_SUPERVISOR, PRI_WARNING,
                     "SipxProcessResource::isReadyToStart returns false; %s is not running ",
                     data());
   }
   return bReady;
}

// Whether or not it is safe to stop a SipxProcess using the SipxProcessResource.
bool SipxProcessResource::isSafeToStop()
{
   UtlString tempStr;
   return ! isReadyToStart(tempStr);
}

// Determine whether or not the values in a containable are comparable.
UtlContainableType SipxProcessResource::getContainableType() const
{
   return TYPE;
}

/// constructor
SipxProcessResource::SipxProcessResource(const char* uniqueId) :
   SipxResource(uniqueId)
{
   mAccess=ReadAccess;
}

/// destructor
SipxProcessResource::~SipxProcessResource()
{
}
