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

#include "FileResourceManager.h"
#include "FileResource.h"

// DEFINES
// CONSTANTS
const UtlContainableType FileResource::TYPE = "FileResource";

const char* FileResource::FileResourceTypeName = "file";
const char* FileResource::OsconfigdbResourceTypeName = "osconfigdb";

// TYPEDEFS
// FORWARD DECLARATIONS

// Factory method that parses a 'file' or 'osconfigdb' resource description element.
bool FileResource::parse(const TiXmlDocument& fileDefinitionDoc, ///< process definition document
                         TiXmlElement* resourceElement, // 'file' or 'osconfigdb' element
                         Process* currentProcess        // whose resources are being read.
                         )
{
   /*
    * This is called by SipxResource::parse with any 'file' or 'osconfigdb' child of
    * the 'resources' element in a process definition.
    *
    * @returns NULL if the element was in any way invalid.
    */
   UtlString errorMsg;
   bool resourceIsValid;

   UtlString path;
   resourceIsValid = textContent(path, resourceElement);
   if (resourceIsValid)
   {
      if (!path.isNull())
      {
         FileResourceManager* fileResourceMgr = FileResourceManager::getInstance();

         FileResource* fileResource;
         if (!(fileResource = fileResourceMgr->find(path)))
         {
            fileResource = new FileResource(path);
         }

         for ( const TiXmlAttribute* attribute = resourceElement->FirstAttribute();
               resourceIsValid && attribute;
               attribute = attribute->Next()
              )
         {
            if (!(resourceIsValid =
                  fileResource->SipxResource::parseAttribute(fileDefinitionDoc, attribute)))
            {
               OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "FileResource::parse "
                             "invalid attribute '%s'",
                             attribute->Name());
            }
         }

         if ( fileResource->mFirstDefinition ) // existing resources are in the manager already
         {
            if (resourceIsValid)
            {
               fileResource->mFirstDefinition = false;
               fileResourceMgr->save(fileResource);
            }
            else
            {
               delete fileResource;
            }
         }
      }
      else
      {
         resourceIsValid = false;
         XmlErrorMsg(fileDefinitionDoc, errorMsg);
         OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "FileResource::parse "
                       "file element is empty %s",
                       errorMsg.data());
      }
   }
   else
   {
      XmlErrorMsg(fileDefinitionDoc, errorMsg);
      OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "FileResource::parse "
                    "invalid content in file element %s",
                    errorMsg.data());
   }

   return resourceIsValid;
}

/// Log files are resources too - this creates a log file resource
FileResource* FileResource::logFileResource(const UtlString& logFilePath, Process* currentProcess)
{
   // a log file resource is read-only and not required
   FileResource* logFile = new FileResource(logFilePath.data());
   logFile->mWritableImplicit = false;
   logFile->mWritable = false;

   logFile->mFirstDefinition = false;

   //@TODO logFile->mUsedBy.append(currentProcess);

   FileResourceManager* fileResourceMgr = FileResourceManager::getInstance();
   fileResourceMgr->save(logFile);

   return logFile;
}



// get a description of the FileResource (for use in logging)
void FileResource::appendDescription(UtlString&  description /**< returned description */)
{
   description.append("file '");
   description.append(data());
   description.append("'");
}


// Whether or not the FileResource is ready for use by a Process.
bool FileResource::isReadyToStart()
{
   return false; // @TODO
}

// Determine whether or not the values in a containable are comparable.
UtlContainableType FileResource::getContainableType() const
{
   return TYPE;
}

/// constructor
FileResource::FileResource(const char* uniqueId) :
   SipxResource(uniqueId)
{
}

/// destructor
FileResource::~FileResource()
{

}


