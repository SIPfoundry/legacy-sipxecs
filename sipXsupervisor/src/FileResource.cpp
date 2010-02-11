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

#include "SipxProcess.h"
#include "FileResourceManager.h"
#include "FileResource.h"

// DEFINES
// CONSTANTS
const UtlContainableType FileResource::TYPE = "FileResource";

const char* FileResource::FileResourceTypeName = "file";
const char* FileResource::OsconfigResourceTypeName = "osconfig";

// TYPEDEFS
// FORWARD DECLARATIONS

// Factory method that parses a 'file' or 'osconfig' resource description element.
bool FileResource::parse(const TiXmlDocument& fileDefinitionDoc, ///< process definition document
                         TiXmlElement* resourceElement, // 'file' or 'osconfig' element
                         SipxProcess* currentProcess        // whose resources are being read.
                         )
{
   /*
    * This is called by SipxResource::parse with any 'file' or 'osconfig' child of
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
         if (!(fileResource = fileResourceMgr->find(path,
                                                    FileResourceManager::RequireExactFileMatch)))
         {
            fileResource = new FileResource(path);
         }

         fileResource->usedBy(currentProcess);

         for ( const TiXmlAttribute* attribute = resourceElement->FirstAttribute();
               resourceIsValid && attribute;
               attribute = attribute->Next()
              )
         {
            if (!(resourceIsValid =
                  fileResource->SipxResource::parseAttribute(fileDefinitionDoc,
                                                             attribute, currentProcess)
                  ))
            {
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "FileResource::parse "
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
               currentProcess->resourceIsOptional(fileResource); // get off the required list
               delete fileResource;
               fileResource = NULL;
            }
         }
      }
      else
      {
         resourceIsValid = false;
         XmlErrorMsg(fileDefinitionDoc, errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "FileResource::parse "
                       "file element is empty %s",
                       errorMsg.data());
      }
   }
   else
   {
      XmlErrorMsg(fileDefinitionDoc, errorMsg);
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "FileResource::parse "
                    "invalid content in file element %s",
                    errorMsg.data());
   }

   return resourceIsValid;
}

/// Log files are resources too - this creates a log file resource
FileResource* FileResource::logFileResource(const UtlString& logFilePath, SipxProcess* currentProcess)
{
   // Check to see if it has already been declared as a resource.
   FileResourceManager* fileResourceMgr = FileResourceManager::getInstance();
   FileResource*        logFile;
   if ( !(logFile = fileResourceMgr->find(logFilePath,
                                          FileResourceManager::RequireExactFileMatch)))
   {
      // a log file resource is read-only and not required
      logFile = new FileResource(logFilePath.data());
      logFile->usedBy(currentProcess);
      currentProcess->resourceIsOptional(logFile); // logs are never required

      logFile->mImplicitAccess = false;
      logFile->mAccess = ReadAccess;

      logFile->mFirstDefinition = false;

      fileResourceMgr->save(logFile);
   }

   return logFile;
}



// get a description of the FileResource (for use in logging)
void FileResource::appendDescription(UtlString&  description /**< returned description */) const
{
   description.append("file '");
   description.append(data());
   description.append("'");
}


// Whether or not the FileResource is ready for use by a SipxProcess.
bool FileResource::isReadyToStart(UtlString& missingResource)
{
   OsPath filePath(*this);
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                 "FileResource::isReadyToStart checking for existence of %s",
                 data());
   bool bReady = OsFileSystem::exists(filePath);
   if ( !bReady )
   {
      missingResource = "";
      appendDescription(missingResource);
   }
   return bReady;
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


