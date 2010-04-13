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
                          SipxProcess* currentProcess        // whose resources are being read.
                          )
{
   /*
    * This is called by SipxResource::parse with any 'sqldb' child of
    * the 'resources' element in a sqldb definition.
    *
    * @returns NULL if the element was in any way invalid.
    */
   UtlString errorMsg;
   bool resourceIsValid = true;
   bool validResourceParm;

   TiXmlElement*    dbElement;

   UtlString databaseName;
   UtlString serverName;
   UtlString userName;
   UtlString dbDriver;
   UtlString userPassword;

   SqldbResource* sqldbResource;
   SqldbResourceManager* sqldbManager = SqldbResourceManager::getInstance();

   dbElement = resourceElement->FirstChildElement();
   if (dbElement)
   {
      if (0 == strcmp("server",dbElement->Value()))
      {
         validResourceParm = textContent(serverName, dbElement);
         if (validResourceParm && !serverName.isNull())
         {
            // advance to the next element, if any.
            dbElement = dbElement->NextSiblingElement();
         }
         else
         {
            resourceIsValid = false;
            XmlErrorMsg(sqldbDefinitionDoc, errorMsg);
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SqldbResource::parse "
                          "'server' element is empty"
                          " - if present, it must be a machine name or localhost %s",
                          errorMsg.data()
                          );
         }
      }
      else
      {
         // 'server' is an optional element.  Setup the server to use the default of localhost
         serverName = "localhost";
      }
    }
    else
    {
       resourceIsValid = false;
       XmlErrorMsg(sqldbDefinitionDoc, errorMsg);
       OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SqldbResource::parse "
                          "no elements are present %s",
                          errorMsg.data()
                          );
    }

    if (resourceIsValid && dbElement)
    {
       if (0 == strcmp("dbname",dbElement->Value()))
       {
          validResourceParm = textContent(databaseName, dbElement);
          if (validResourceParm && !databaseName.isNull())
          {
             // advance to the next element, if any.
             dbElement = dbElement->NextSiblingElement();
          }
          else
          {
             resourceIsValid = false;
             XmlErrorMsg(sqldbDefinitionDoc, errorMsg);
             OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SqldbResource::parse "
                           "'dbname' element is empty"
                           " - if present it must contain a valid sql database name"
                           );
          }
       }
       else
       {
          resourceIsValid = false;
          XmlErrorMsg(sqldbDefinitionDoc, errorMsg);
          OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SqldbResource::parse "
                        "'dbname' element is missing %s",
                        errorMsg.data()
                        );
       }
    }

    if (resourceIsValid)
    {
       if (dbElement && (0 == strcmp("username",dbElement->Value())))
       {
          validResourceParm = textContent(userName, dbElement);
          if (validResourceParm && !userName.isNull())
          {
             // advance to the next element, if any.
             dbElement = dbElement->NextSiblingElement();
          }
          else
          {
             resourceIsValid = false;
             XmlErrorMsg(sqldbDefinitionDoc, errorMsg);
             OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SqldbResource::parse "
                           "'username' element is empty"
                           " - if present, it must be a valid database user name %s",
                           errorMsg.data()
                           );
          }
       }
       else
       {
          // 'username' is an optional element.  Setup the username to use the default of postgres
          userName = "postgres";
       }
    }

    if (resourceIsValid)
    {
       if (dbElement && (0 == strcmp("dbdriver",dbElement->Value())))
       {
          validResourceParm = textContent(dbDriver, dbElement);
          if (validResourceParm && !dbDriver.isNull())
          {
             // advance to the next element, if any.
             dbElement = dbElement->NextSiblingElement();
          }
          else
          {
             resourceIsValid = false;
             XmlErrorMsg(sqldbDefinitionDoc, errorMsg);
             OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SqldbResource::parse "
                           "'dbdriver' element is empty"
                           " - if present, it must be a valid database driver name %s",
                           errorMsg.data()
                           );
          }
       }
       else
       {
          /* 'dbdriver' is an optional element.
           * Setup the database driver to use the default of {PostgreSQL} */
          dbDriver = "{PostgreSQL}";
       }
    }
    if (resourceIsValid)
    {
       if (dbElement && (0 == strcmp("userpassword",dbElement->Value())))
       {
          validResourceParm = textContent(userPassword, dbElement);
          if (!validResourceParm || userPassword.isNull())
          {
             resourceIsValid = false;
             XmlErrorMsg(sqldbDefinitionDoc, errorMsg);
             OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SqldbResource::parse "
                           "'userpassword' element is empty"
                           " - if present, it must be a valid password string %s",
                           errorMsg.data()
                           );
          }
       }
       else
       {
          // 'userpassword' is an optional element.  Setup the password to be an empty string
          userPassword = "";
       }
    }

    if (resourceIsValid)
    {
       if (!(sqldbResource = sqldbManager->find(databaseName + serverName + userName)))
       {
          sqldbResource = new SqldbResource(databaseName + serverName + userName );
       }
       sqldbResource->usedBy(currentProcess);

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
             OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SqldbResource::parse "
                           "invalid attribute '%s'",
                           attribute->Name());
          }
       }

       if ( sqldbResource->mFirstDefinition ) // existing resources are in the manager already
       {
          if (resourceIsValid)
          {
             sqldbResource->mFirstDefinition = false;
             sqldbResource->mUser = userName;
             sqldbResource->mPassword = userPassword;
             sqldbResource->mDbDriver = dbDriver;
             sqldbResource->mServer = serverName;
             sqldbResource->mDbName = databaseName;
             OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE, "SqldbResource::parse "
                                  "databaseName = %s, username = %s, password = %s, driver = %s, server = %s",
                                  sqldbResource->mDbName.data(),
                                  sqldbResource->mUser.data(),
                                  sqldbResource->mPassword.data(),
                                  sqldbResource->mDbDriver.data(),
                                  sqldbResource->mServer.data()
                                  );
             sqldbManager->save(sqldbResource);
          }
       }
       else
       {
          delete sqldbResource;
       }

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
bool SqldbResource::isReadyToStart(UtlString& missingResource)
{
   bool dbIsReady;

   // Check to ensure that we can connect to the database.
   OdbcHandle dbHandle = odbcConnect(mDbName, mServer, mUser, mDbDriver, mPassword);
   if (dbHandle)
   {
      odbcDisconnect(dbHandle);
      OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "SqldbResource::isReadyToStart "
                     "Successfully connected to database '%s'",
                     mDbName.data());
      dbIsReady = true;
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SqldbResource::isReadyToStart "
                     "Unable to connect to database '%s'",
                     mDbName.data());
      missingResource = "";
      appendDescription(missingResource);
      dbIsReady = false;
   }

   return dbIsReady;
}

// Determine whether or not the values in a containable are comparable.
UtlContainableType SqldbResource::getContainableType() const
{
   return TYPE;
}

/// constructor
SqldbResource::SqldbResource(const char* uniqueId) :
   SipxResource(uniqueId)
{
}


/// destructor
SqldbResource::~SqldbResource()
{
}
