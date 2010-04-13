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

#include "sipdb/SIPDBManager.h"

#include "ImdbResourceManager.h"
#include "ImdbResource.h"

// DEFINES
// CONSTANTS
const UtlContainableType ImdbResource::TYPE = "ImdbResource";

const char* ImdbResource::ImdbResourceTypeName = "imdb";

// STATICS
// FORWARD DECLARATIONS

// Factory method that parses a 'imdb' resource description element.
bool ImdbResource::parse(const TiXmlDocument& imdbDefinitionDoc, ///< imdb definition document
                         TiXmlElement* resourceElement, // 'imdb' element
                         SipxProcess* currentProcess        // whose resources are being read.
                         )
{
   /*
    * This is called by SipxResource::parse with any 'imdb' child of
    * the 'resources' element in a imdb definition.
    *
    * @returns NULL if the element was in any way invalid.
    */
   UtlString errorMsg;
   bool resourceIsValid;

   UtlString tableName;
   resourceIsValid = textContent(tableName, resourceElement);
   if (resourceIsValid)
   {
      if (!tableName.isNull())
      {
         ImdbResourceManager* imdbResourceMgr = ImdbResourceManager::getInstance();

         ImdbResource* imdbResource;
         if (!(imdbResource = imdbResourceMgr->find(tableName)))
         {
            imdbResource = new ImdbResource(tableName);
         }
         imdbResource->usedBy(currentProcess);

         for ( const TiXmlAttribute* attribute = resourceElement->FirstAttribute();
               resourceIsValid && attribute;
               attribute = attribute->Next()
              )
         {
            if (!(resourceIsValid =
                  imdbResource->SipxResource::parseAttribute(imdbDefinitionDoc,
                                                             attribute, currentProcess)
                  ))
            {
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "ImdbResource::parse "
                             "invalid attribute '%s'",
                             attribute->Name());
            }
         }

         if ( imdbResource->mFirstDefinition ) // existing resources are in the manager already
         {
            if (resourceIsValid)
            {
               imdbResource->mFirstDefinition = false;
               imdbResourceMgr->save(imdbResource);
            }
            else
            {
               delete imdbResource;
            }
         }
      }
      else
      {
         resourceIsValid = false;
         XmlErrorMsg(imdbDefinitionDoc, errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "ImdbResource::parse "
                       "imdb element is empty %s",
                       errorMsg.data());
      }
   }
   else
   {
      XmlErrorMsg(imdbDefinitionDoc, errorMsg);
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "ImdbResource::parse "
                    "invalid content in imdb element %s",
                    errorMsg.data());
   }

   return resourceIsValid;
}


// get a description of the ImdbResource (for use in logging)
void ImdbResource::appendDescription(UtlString&  description /**< returned description */) const
{
   description.append("imdb '");
   description.append(data());
   description.append("'");
}


// Whether or not the ImdbResource is ready for use by a Imdb.
bool ImdbResource::isReadyToStart(UtlString& missingResource)
{
   OsLock mutex(mLock);

   bool rc;
   rc = false;


   if (NULL == mDatabase)
   {
        OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                      "Getting database '%s' returning missing resource", this->data());
        mDatabase = SIPDBManager::getInstance()->getDatabase(*this);
   }

   rc = (NULL != mDatabase);
   if (rc)
   {
      // preload the database.  if it fails (i.e. no xml file) then resource not ready.
      if (SIPDBManager::getInstance()->preloadDatabaseTable(*this) != OS_SUCCESS)
      {
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                       "FAILED to load database '%s' returning missing resource", this->data());
         rc = false;
      }
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                    "FAILED to get database '%s' returning missing resource", this->data());

   }
   
   if (!rc)
   {
      missingResource = "";
      appendDescription(missingResource);
   }
   return rc;
}

// Determine whether or not the values in a containable are comparable.
UtlContainableType ImdbResource::getContainableType() const
{
   return TYPE;
}

/// constructor
ImdbResource::ImdbResource(const char* uniqueId) :
   SipxResource(uniqueId),
   mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mDatabase(NULL)
{
}

/// destructor
ImdbResource::~ImdbResource()
{
   OsLock mutex(mLock);
   SIPDBManager::getInstance()->removeDatabase(*this);
   mDatabase = NULL;
}



