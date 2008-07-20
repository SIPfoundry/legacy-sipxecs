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

#include "SqldbResourceManager.h"
#include "SqldbResource.h"

// DEFINES
// CONSTANTS
const UtlContainableType SqldbResource::TYPE = "SqldbResource";

const char* SqldbResource::SqldbResourceTypeName = "sqldb";

// TYPEDEFS
// FORWARD DECLARATIONS

// Factory method that parses a 'sqldb' resource description element.
bool SqldbResource::parse(const TiXmlDocument& sqldbDefinitionDoc, ///< sqldb definition document
                          TiXmlElement* resourceElement, // 'sqldb' element
                          Process* currentProcess        // whose resources are being read.
                          )
{
   /*
    * This is called by SipxResource::parse with any 'sqldb' child of
    * the 'resources' element in a sqldb definition.
    *
    * @returns NULL if the element was in any way invalid.
    */
   UtlString errorMsg;
   bool resourceIsValid;

   UtlString databaseName;
   resourceIsValid = textContent(databaseName, resourceElement);
   if (resourceIsValid)
   {
      if (!databaseName.isNull())
      {
         SqldbResourceManager* sqldbResourceMgr = SqldbResourceManager::getInstance();

         SqldbResource* sqldbResource;
         if (!(sqldbResource = sqldbResourceMgr->find(databaseName)))
         {
            sqldbResource = new SqldbResource(databaseName, currentProcess);
         }

         for ( const TiXmlAttribute* attribute = resourceElement->FirstAttribute();
               resourceIsValid && attribute;
               attribute = attribute->Next()
              )
         {
            if (!(resourceIsValid =
                  sqldbResource->SipxResource::parseAttribute(sqldbDefinitionDoc,
                                                              attribute, currentProcess)
                  ))
            {
               OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "SqldbResource::parse "
                             "invalid attribute '%s'",
                             attribute->Name());
            }
         }

         if ( sqldbResource->mFirstDefinition ) // existing resources are in the manager already
         {
            if (resourceIsValid)
            {
               sqldbResource->mFirstDefinition = false;
               sqldbResourceMgr->save(sqldbResource);
            }
            else
            {
               delete sqldbResource;
            }
         }
      }
      else
      {
         resourceIsValid = false;
         XmlErrorMsg(sqldbDefinitionDoc, errorMsg);
         OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "SqldbResource::parse "
                       "sqldb element is empty %s",
                       errorMsg.data());
      }
   }
   else
   {
      XmlErrorMsg(sqldbDefinitionDoc, errorMsg);
      OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "SqldbResource::parse "
                    "invalid content in sqldb element %s",
                    errorMsg.data());
   }

   return resourceIsValid;
}


// get a description of the SqldbResource (for use in logging)
void SqldbResource::appendDescription(UtlString&  description /**< returned description */) const
{
   description.append("SQL database '");
   description.append(data());
   description.append("'");
}


// Whether or not the SqldbResource is ready for use by a Sqldb.
bool SqldbResource::isReadyToStart()
{
   return false; // @TODO
}

// Determine whether or not the values in a containable are comparable.
UtlContainableType SqldbResource::getContainableType() const
{
   return TYPE;
}

/// constructor
SqldbResource::SqldbResource(const char* uniqueId, Process* currentProcess) :
   SipxResource(uniqueId, currentProcess)
{
}


/// destructor
SqldbResource::~SqldbResource()
{
}
