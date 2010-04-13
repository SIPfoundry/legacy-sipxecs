//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlHashMapIterator.h"
#include "net/XmlRpcDispatch.h"
#include "configrpc/ConfigRPC.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* MethodName[ConfigRPC_Callback::NumMethods] =
{
   "configurationParameter.version",
   "configurationParameter.get",
   "configurationParameter.set",
   "configurationParameter.delete"
};

// STATICS

OsRWMutex* ConfigRPC::spDatabaseLock = new OsRWMutex(OsBSem::Q_PRIORITY);
UtlHashBag ConfigRPC::sDatabases;
bool       ConfigRPC::sRegistered = false;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/// Construct an instance to allow RPC access to a database.
ConfigRPC::ConfigRPC( const char*         dbName     ///< dbName known to XMLRPC methods
                     ,const char*         versionId  ///< version of this database
                     ,const UtlString&    dbPath     ///< path to persistent store for this db
                     ,ConfigRPC_Callback* callback   ///< connection to controlling application
                     )
   :UtlString(dbName)
   ,mVersion(versionId)
   ,mPath(dbPath)
   ,mCallback(callback)
{
   assert(dbName && *dbName != '\000');
   assert(versionId && *versionId != '\000');
   assert(!dbPath.isNull());
   assert(callback);

   OsWriteLock lock(*spDatabaseLock);

   if ( ! sDatabases.find(this) )
   {
      OsSysLog::add( FAC_KERNEL, PRI_INFO, "ConfigRPC:: register access to db name '%s'", dbName);
      sDatabases.insert(this);
   }
   else
   {
      OsSysLog::add( FAC_KERNEL, PRI_CRIT, "ConfigRPC:: duplicate db name '%s'", dbName);
   }
}

OsStatus ConfigRPC::load(OsConfigDb& dataset)
{
   OsStatus status = dataset.loadFromFile(mPath);
   if ( OS_SUCCESS != status )
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "ConfigRPC failed to load '%s' from '%s'",
                    data(), mPath.data()
                    );
   }
   return status;
}


OsStatus ConfigRPC::store(OsConfigDb& dataset)
{
   OsStatus status = dataset.storeToFile(mPath);
   if ( OS_SUCCESS != status )
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "ConfigRPC failed to store '%s' to '%s'",
                    data(), mPath.data()
                    );
   }
   return status;
}

/// Destroy the instance to disconnect access to the database.
ConfigRPC::~ConfigRPC()
{
   OsWriteLock lock(*spDatabaseLock);
   sDatabases.remove(this);
}

/*****************************************************************
 * Default ConfigRPC_Callback
 *****************************************************************/

ConfigRPC_Callback::ConfigRPC_Callback()
{
}

// Access check function
XmlRpcMethod::ExecutionStatus ConfigRPC_Callback::accessAllowed(
   const HttpRequestContext& requestContext,
   Method                    method
                                                                ) const
{
   OsSysLog::add(FAC_KERNEL, PRI_INFO,
                 "ConfigRPC default accessAllowed for %s",
                 MethodName[method]
                 );
   return XmlRpcMethod::OK;
}

/// Invoked after the database has been modified
void ConfigRPC_Callback::modified()
{
   // in the abstract base class this is a no-op
   OsSysLog::add(FAC_KERNEL, PRI_INFO, "ConfigRPC default modified");
}


ConfigRPC_Callback::~ConfigRPC_Callback()
{
}

/// Instantiate this to allow configuration from hosts in the same SIP domain
ConfigRPC_InDomainCallback::ConfigRPC_InDomainCallback(const UtlString& domain ///< to allow
                                                       )
   :mAllowedDomain(domain)
{
}


/// Access check function
XmlRpcMethod::ExecutionStatus ConfigRPC_InDomainCallback::accessAllowed(
   const HttpRequestContext&  requestContext,
   ConfigRPC_Callback::Method method
                                                                        ) const
{
   XmlRpcMethod::ExecutionStatus isAllowed = (  requestContext.isTrustedPeer(mAllowedDomain)
                                              ? XmlRpcMethod::OK
                                              : XmlRpcMethod::FAILED
                                              );
   /*
    * - XmlRpcMethod::OK if allowed
    * - XmlRpcMethod::FAILED if not allowed,
    * - XmlRpcMethod::REQUIRE_AUTHENTICATION if authentication is missing or invalid.
    */
   if (XmlRpcMethod::FAILED == isAllowed)
   {
      OsSysLog::add(FAC_KERNEL, PRI_WARNING,
                    "ConfigRPC_InDomainCallback disallowed configuration from untrusted peer"
                    );
   }
   return isAllowed;
}



/// Implements the XML-RPC method configurationParameter.version
/**
 * Inputs:
 *  string    dbName           Name of the database.
 *  Outputs:
 *  string    version_id       The version identifier.
 *
 *  This method should be called by a configuration application to
 *  confirm that it is using a compatible definition of the database
 *  definition.  If the returned version_id does not match what the
 *  configuration application expects, it should not modify the configuration..
 */
class ConfigRPC_version : public XmlRpcMethod
{
public:
   static XmlRpcMethod* get()
      {
         return new ConfigRPC_version();
      }

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        )
      {
         UtlString* dbName = dynamic_cast<UtlString*>(params.at(0));

         if (dbName && !dbName->isNull())
         {
            OsReadLock lock(*ConfigRPC::spDatabaseLock);
            ConfigRPC* db = ConfigRPC::find(*dbName);

            if (db)
            {
               status = db->mCallback->accessAllowed(requestContext, ConfigRPC_Callback::Version);
               if ( XmlRpcMethod::OK == status )
               {
                  response.setResponse(&db->mVersion);
               }
               else
               {
                  UtlString faultMsg("Access Denied");
                  response.setFault(XmlRpcMethod::FAILED, faultMsg.data());
               }
            }
            else
            {
               UtlString faultMsg;
               faultMsg.append("db lookup failed for '");
               faultMsg.append(*dbName);
               faultMsg.append("'");
               response.setFault( XmlRpcResponse::UnregisteredMethod, faultMsg.data());
               status = XmlRpcMethod::FAILED;
            }
         }
         else
         {
            response.setFault( XmlRpcResponse::EmptyParameterValue
                              ,"'dbname' parameter is missing or invalid type"
                              );
            status = XmlRpcMethod::FAILED;
         }

         return true;
      }
};


/// Implements the XML-RPC method configurationParameter.get
/*
 * Parameters  Type      Name             Description
 *  Inputs:
 *             string    db_name          configuration data set name
 *             array
 *               string  parameter        name of parameter to return
 *  Outputs:
 *             struct
 *               string  parameter        parameter name (key)
 *               string  value            parameter value
 *
 * Returns the name and value for each parameter in the input array
 * of parameter names.  If any parameter in the set is undefined, a
 * PARAMETER_UNDEFINED fault is returned.
 *
 * To get all the parameters in the database, call this method with just the db_name.
 * When called with just the db_name, if the dataset is empty (there are no parameters
 * defined), a DATASET_EMPTY fault is returned.
 */
class ConfigRPC_get : public XmlRpcMethod
{
public:

   static XmlRpcMethod* get()
      {
         return new ConfigRPC_get();
      }

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        )
      {
         UtlString* dbName = dynamic_cast<UtlString*>(params.at(0));

         if (dbName && !dbName->isNull())
         {
            OsReadLock lock(*ConfigRPC::spDatabaseLock);

            // find the dataset registered with this name
            ConfigRPC* db = ConfigRPC::find(*dbName);
            if (db)
            {
               // check with the application to see if this request is authorized on this dataset
               status = db->mCallback->accessAllowed(requestContext, ConfigRPC_Callback::Get);
               if ( XmlRpcMethod::OK == status )
               {
                  // read in the dataset
                  OsConfigDb dataset;
                  OsStatus datasetStatus = db->load(dataset);
                  if ( OS_SUCCESS == datasetStatus )
                  {
                     // get the list of names that the request is asking for
                     UtlContainable* secondParam = params.at(1);
                     if ( secondParam )
                     {
                        UtlSList* nameList = dynamic_cast<UtlSList*>(secondParam);
                        if (nameList)
                        {
                           /*
                            * Iterate over the requested names
                            * - All must be present or the request is an error
                            * - For each name found, add the name and value to the
                            *   selectedParams hash to be returned in a success response.
                            */
                           UtlHashMap selectedParams;
                           UtlSListIterator requestedNames(*nameList);
                           UtlString* requestedName = NULL;
                           bool allNamesFound = true;

                           while (   allNamesFound
                                  && (requestedName = dynamic_cast<UtlString*>(requestedNames()))
                                  )
                           {
                              UtlString* paramValue = new UtlString();
                              if ( OS_SUCCESS == dataset.get(*requestedName, *paramValue) )
                              {
                                 UtlString* paramName  = new UtlString(*requestedName);
                                 // put it into the results
                                 selectedParams.insertKeyAndValue(paramName, paramValue);
                              }
                              else
                              {
                                 allNamesFound = false;
                                 delete paramValue;
                              }
                           }

                           if (allNamesFound)
                           {
                              // all were found - return the name/value pairs
                              response.setResponse(&selectedParams);
                           }
                           else
                           {
                              // at least one name was not found - return an error.
                              UtlString faultMsg;
                              faultMsg.append("parameter name '");
                              faultMsg.append(*requestedName);
                              faultMsg.append("' not found");
                              response.setFault(ConfigRPC::nameNotFound, faultMsg.data());
                              status = XmlRpcMethod::FAILED;
                           }

                           selectedParams.destroyAll();
                        }
                        else
                        {
                           // The second parameter was not a list
                           response.setFault( ConfigRPC::invalidType
                                             ,"namelist parameter is not an array"
                                             );
                           status = XmlRpcMethod::FAILED;
                        }
                     }
                     else // no parameter names specified
                     {
                        // return all names
                        UtlHashMap allParams;
                        UtlString  lastKey;
                        OsStatus   iterateStatus;
                        UtlString* paramName;
                        UtlString* paramValue;
                        bool       notEmpty = false;

                        for ( ( paramName  = new UtlString()
                               ,paramValue = new UtlString()
                               ,iterateStatus = dataset.getNext(lastKey, *paramName, *paramValue)
                               );
                              OS_SUCCESS == iterateStatus;
                              ( lastKey       = *paramName
                               ,paramName     = new UtlString()
                               ,paramValue    = new UtlString()
                               ,iterateStatus = dataset.getNext(lastKey, *paramName, *paramValue)
                               )
                             )
                        {
                           notEmpty = true; // got at least one parameter
                           // put it into the result array
                           allParams.insertKeyAndValue(paramName, paramValue);
                        }
                        // on the final iteration these were not used
                        delete paramName;
                        delete paramValue;

                        if (notEmpty)
                        {
                           response.setResponse(&allParams);
                           allParams.destroyAll();
                        }
                        else
                        {
                           // there is no way to send a well-formed but empty response,
                           // so a 'get all' on an empty dataset returns a fault.
                           UtlString faultMsg;
                           faultMsg.append("dataset '");
                           faultMsg.append(*dbName);
                           faultMsg.append("' has no parameters");
                           response.setFault(ConfigRPC::emptyDataset, faultMsg);
                           status = XmlRpcMethod::FAILED;
                        }
                     }
                  }
                  else
                  {
                     UtlString faultMsg("dataset load failed");
                     response.setFault(ConfigRPC::loadFailed, faultMsg);
                     status = XmlRpcMethod::FAILED;
                  }
               }
               else
               {
                  UtlString faultMsg("Access Denied");
                  response.setFault(XmlRpcMethod::FAILED, faultMsg.data());
               }
            }
            else
            {
               UtlString faultMsg;
               faultMsg.append("db lookup failed for '");
               faultMsg.append(*dbName);
               faultMsg.append("'");
               response.setFault( XmlRpcResponse::UnregisteredMethod, faultMsg.data());
               status = XmlRpcMethod::FAILED;
            }
         }
         else
         {
            response.setFault( XmlRpcResponse::EmptyParameterValue
                              ,"'dbname' parameter is missing or invalid type"
                              );
            status = XmlRpcMethod::FAILED;
         }

         return true;
      }
};

/// Implements the XML-RPC method configurationParameter.set method
/**
 *  Parameters  Type      Name             Description
 *  Inputs:
 *              string    db_name          configuration data set name
 *              struct
 *                string  parameter      parameter name (key)
 *                string  value          parameter value
 *    ...
 *  Outputs:
 *              integer                    number of values set
 *
 *  Sets each 'parameter' / 'value' pair in 'db_name'.  Either all
 *  sets are made or none are made.
 */
class ConfigRPC_set : public XmlRpcMethod
{
public:
   static XmlRpcMethod* get()
      {
         return new ConfigRPC_set();
      }

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        )
      {
         UtlString* dbName = dynamic_cast<UtlString*>(params.at(0));

         if (dbName && !dbName->isNull())
         {
            OsReadLock lock(*ConfigRPC::spDatabaseLock);

            ConfigRPC* db = ConfigRPC::find(*dbName);
            if (db)
            {
               status = db->mCallback->accessAllowed(requestContext, ConfigRPC_Callback::Set);
               if ( XmlRpcMethod::OK == status )
               {
                  // read in the dataset
                  OsConfigDb dataset;
                  OsStatus datasetStatus = db->load(dataset);
                  if ( OS_SUCCESS == datasetStatus )
                  {
                     // get the list of names that the request is asking for
                     UtlContainable* secondParam = params.at(1);
                     if ( secondParam )
                     {
                        UtlHashMap* paramList = dynamic_cast<UtlHashMap*>(secondParam);
                        if (paramList)
                        {
                           /*
                            * Iterate over the requested name/value pairs
                            */
                           UtlHashMapIterator params(*paramList);
                           UtlContainable* nextParam = NULL;
                           size_t paramsSet = 0;

                           while (    XmlRpcMethod::OK == status
                                  && (nextParam = params())
                                  )
                           {
                              UtlString* name = dynamic_cast<UtlString*>(params.key());
                              if ( name )
                              {
                                 UtlString* value = dynamic_cast<UtlString*>(params.value());
                                 if (value)
                                 {
                                    dataset.set(*name, *value);
                                    paramsSet++;
                                 }
                                 else
                                 {
                                    UtlString faultMsg;
                                    faultMsg.append("parameter name '");
                                    faultMsg.append(*name);
                                    faultMsg.append("' value is not a string");
                                    response.setFault(ConfigRPC::invalidType, faultMsg.data());
                                    status = XmlRpcMethod::FAILED;
                                 }
                              }
                              else
                              {
                                 UtlString faultMsg;
                                 faultMsg.append("parameter number ");
                                 char paramIndex[10];
                                 sprintf(paramIndex,"%zu", paramsSet + 1);
                                 faultMsg.append(paramIndex);
                                 faultMsg.append(" name is not a string");
                                 response.setFault(ConfigRPC::invalidType, faultMsg.data());
                                 status = XmlRpcMethod::FAILED;
                              }
                           }

                           if ( XmlRpcMethod::OK == status )
                           {
                              if (OS_SUCCESS == db->store(dataset))
                              {
                                 UtlInt numberSet(paramList->entries());
                                 response.setResponse(&numberSet);
                              }
                              else
                              {
                                 response.setFault( ConfigRPC::storeFailed
                                                   ,"error storing dataset"
                                                   );
                                 status = XmlRpcMethod::FAILED;
                              }
                           }
                        }
                        else
                        {
                           // The second parameter was not a list
                           response.setFault( ConfigRPC::invalidType
                                             ,"second parameter is not a struct"
                                             );
                           status = XmlRpcMethod::FAILED;
                        }
                     }
                     else // no parameter names specified
                     {
                        // No second parameter
                        response.setFault( ConfigRPC::invalidType
                                          ,"no second parameter of name/value pairs"
                                          );
                        status = XmlRpcMethod::FAILED;
                     }
                  }
                  else
                  {
                     UtlString faultMsg("dataset load failed");
                     response.setFault(ConfigRPC::loadFailed, faultMsg);
                     status = XmlRpcMethod::FAILED;
                  }
               }
               else
               {
                  UtlString faultMsg("Access Denied");
                  response.setFault(XmlRpcMethod::FAILED, faultMsg.data());
               }
            }
            else
            {
               UtlString faultMsg;
               faultMsg.append("db lookup failed for '");
               faultMsg.append(*dbName);
               faultMsg.append("'");
               response.setFault( XmlRpcResponse::UnregisteredMethod, faultMsg.data());
               status = XmlRpcMethod::FAILED;
            }
         }
         else
         {
            response.setFault( XmlRpcResponse::EmptyParameterValue
                              ,"'dbname' parameter is missing or invalid type"
                              );
            status = XmlRpcMethod::FAILED;
         }

         return true;
      }
};

/*

  Method: configurationParameter.delete

  Parameters  Type      Name             Description
  Inputs:
  string    db_name          configuration data set name
  string    parameter        parameter name (key)
  Outputs:
  (none)

  Removes 'parameter' from 'db_name'.  This causes the value of
  'parameter' to be undefined.  It is not an error to invoke the
  delete method on an undefined parameter.

*/
class ConfigRPC_delete : public XmlRpcMethod
{
public:
   static XmlRpcMethod* get()
      {
         return new ConfigRPC_delete();
      }

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        )
      {
         UtlString* dbName = dynamic_cast<UtlString*>(params.at(0));

         if (dbName && !dbName->isNull())
         {
            OsReadLock lock(*ConfigRPC::spDatabaseLock);

            ConfigRPC* db = ConfigRPC::find(*dbName);
            if (db)
            {
               status = db->mCallback->accessAllowed(requestContext, ConfigRPC_Callback::Set);
               if ( XmlRpcMethod::OK == status )
               {
                  // read in the dataset
                  OsConfigDb dataset;
                  OsStatus datasetStatus = db->load(dataset);
                  if ( OS_SUCCESS == datasetStatus )
                  {
                     // get the list of names that the request is trying to delete
                     UtlContainable* secondParam = params.at(1);
                     if ( secondParam )
                     {
                        UtlSList* nameList = dynamic_cast<UtlSList*>(secondParam);
                        if (nameList)
                        {
                           /*
                            * Iterate over the names
                            * - For each name found, delete it from the dataset and count it
                            */
                           UtlSListIterator deleteNames(*nameList);
                           UtlString* deleteName = NULL;
                           size_t deleted = 0;

                           while ((deleteName = dynamic_cast<UtlString*>(deleteNames())))
                           {
                              if (OS_SUCCESS == dataset.remove(*deleteName))
                              {
                                 deleted++;
                              }
                           }

                           if (OS_SUCCESS == db->store(dataset))
                           {
                              status = XmlRpcMethod::OK;
                              UtlInt deletedCount(deleted);
                              response.setResponse(&deletedCount);
                           }
                           else
                           {
                              response.setFault( ConfigRPC::storeFailed
                                                ,"error storing dataset"
                                                );
                              status = XmlRpcMethod::FAILED;
                           }
                        }
                        else
                        {
                           // The second parameter was not a list
                           response.setFault( ConfigRPC::invalidType
                                             ,"namelist parameter is not an array"
                                             );
                           status = XmlRpcMethod::FAILED;
                        }
                     }
                     else // No second parameter
                     {
                        response.setFault( ConfigRPC::invalidType
                                          ,"no second parameter list of names to delete"
                                          );
                        status = XmlRpcMethod::FAILED;
                     }
                  }
                  else
                  {
                     UtlString faultMsg("dataset load failed");
                     response.setFault(ConfigRPC::loadFailed, faultMsg);
                     status = XmlRpcMethod::FAILED;
                  }
               }
               else
               {
                  UtlString faultMsg("Access Denied");
                  response.setFault(XmlRpcMethod::FAILED, faultMsg.data());
               }
            }
            else
            {
               UtlString faultMsg;
               faultMsg.append("db lookup failed for '");
               faultMsg.append(*dbName);
               faultMsg.append("'");
               response.setFault( XmlRpcResponse::UnregisteredMethod, faultMsg.data());
               status = XmlRpcMethod::FAILED;
            }
         }
         else
         {
            response.setFault( XmlRpcResponse::EmptyParameterValue
                              ,"'dbname' parameter is missing or invalid type"
                              );
            status = XmlRpcMethod::FAILED;
         }

         return true;
      }
};


// Must be called once to connect the configurationParameter methods
void ConfigRPC::registerMethods(XmlRpcDispatch&     rpc /* xmlrpc dispatch service to use */)
{
   OsWriteLock lock(*spDatabaseLock);

   if (!sRegistered)
   {
      rpc.addMethod(MethodName[ConfigRPC_Callback::Version], ConfigRPC_version::get, NULL);
      rpc.addMethod(MethodName[ConfigRPC_Callback::Get],     ConfigRPC_get::get,     NULL);
      rpc.addMethod(MethodName[ConfigRPC_Callback::Set],     ConfigRPC_set::get,     NULL);
      rpc.addMethod(MethodName[ConfigRPC_Callback::Delete],  ConfigRPC_delete::get,  NULL);

      sRegistered = true;
   }
}
