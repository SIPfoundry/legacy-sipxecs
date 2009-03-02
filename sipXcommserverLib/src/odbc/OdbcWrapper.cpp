//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <string.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/UtlString.h>
#include "odbc/OdbcWrapper.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATICS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

OdbcHandle odbcConnect(const char* dbname,
                       const char* servername,
                       const char* username,
                       const char* driver,                       
                       const char* password)

{
   OdbcHandle handle = new OdbcControlStruct;
   
   if (handle)
   {
      if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, 
                                        SQL_NULL_HANDLE, 
                                        &handle->mEnvironmentHandle)))
      {
         OsSysLog::add(FAC_ODBC, PRI_ERR,
                       "odbcConnect: Failed to allocate "
                       "environment handle");
         delete handle;
         handle = NULL;
      }
      else 
      {
         SQLSetEnvAttr(handle->mEnvironmentHandle, SQL_ATTR_ODBC_VERSION, 
                       (void*) SQL_OV_ODBC3, 0);
         
         if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, 
                                            handle->mEnvironmentHandle, 
                                            &handle->mConnectionHandle)))
         {
            OsSysLog::add(FAC_ODBC, PRI_ERR,
                          "odbcConnect: Failed to allocate "
                          "connection handle");
            SQLFreeHandle(SQL_HANDLE_ENV, handle->mEnvironmentHandle);
            delete handle;
            handle = NULL;
         }
         else 
         {
            // Connect - contruct connection string
            UtlString connectionString;
            char temp[128];
            
            if (dbname)
            {
               sprintf(temp, "DATABASE=%s;", dbname);
               connectionString.append(temp);
            }
            if (servername)
            {
               sprintf(temp, "SERVER=%s;", servername);
               connectionString.append(temp);
            }            
            if (username)
            {
               sprintf(temp, "UID=%s;", username);
               connectionString.append(temp);
            }            
            if (password)
            {
               sprintf(temp, "PWD=%s;", password);
               connectionString.append(temp);
            }
            else
            {
               connectionString.append("PWD=;");
            }
            if (driver)
            {
               sprintf(temp, "DRIVER=%s;", driver);
               connectionString.append(temp);
            }  
            // Connect in read/write mode
            connectionString.append("READONLY=0;");
           
            SQLRETURN sqlRet = 
                  SQLDriverConnect(handle->mConnectionHandle,
                                   NULL,
                                   (SQLCHAR*)connectionString.data(),
                                   SQL_NTS,
                                   NULL,
                                   0,
                                   NULL,
                                   SQL_DRIVER_NOPROMPT);
                                   
            if (!SQL_SUCCEEDED(sqlRet))
            {
               OsSysLog::add(FAC_ODBC, PRI_ERR,
                             "odbcConnect: Failed to connect %s, error code %d",
                             connectionString.data(), sqlRet);   
                             
               SQLFreeHandle(SQL_HANDLE_DBC, handle->mConnectionHandle);
               SQLFreeHandle(SQL_HANDLE_ENV, handle->mEnvironmentHandle);
               delete handle;
               handle = NULL;                 
            }
            else
            {
               // Connected to database, now alloc a statement handle
               if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT,
                                                handle->mConnectionHandle,
                                                &handle->mStatementHandle)))
               {
                  OsSysLog::add(FAC_ODBC, PRI_ERR,
                                "odbcConnect: Failed to allocate "
                                "statement handle");
                  
                  // Disconnect from database
                  SQLDisconnect(handle->mConnectionHandle);
                  // Free resources
                  
                  SQLFreeHandle(SQL_HANDLE_DBC, handle->mConnectionHandle);
                  SQLFreeHandle(SQL_HANDLE_ENV, handle->mEnvironmentHandle);
                  delete handle;
                  handle = NULL;  
               }
               else
               {              
                  OsSysLog::add(FAC_ODBC, PRI_DEBUG,
                                "odbcConnect: Connected to database "
                                "%s, OdbcHandle %p", 
                                connectionString.data(), handle);
                }
            }
         }
      }
   }
   else
   {
      OsSysLog::add(FAC_ODBC, PRI_ERR,
                    "odbcConnect: Couldn't create OdbcHandle");
   }
   
   return handle;
}
                        
bool odbcDisconnect(OdbcHandle &handle)
{
   bool ret = false;
   
   if (handle)
   {
      SQLRETURN sqlRet;
            
      SQLFreeStmt(handle->mStatementHandle, SQL_CLOSE);
      
      sqlRet = SQLDisconnect(handle->mConnectionHandle);
      if (!SQL_SUCCEEDED(sqlRet))
      {
         OsSysLog::add(FAC_ODBC, PRI_ERR,
                       "odbcDisconnect - failed disconnecting from "
                       "database, error code %d", sqlRet);
      }
      else
      {
         SQLFreeHandle(SQL_HANDLE_STMT, handle->mStatementHandle);
         SQLFreeHandle(SQL_HANDLE_DBC, handle->mConnectionHandle);
         SQLFreeHandle(SQL_HANDLE_ENV, handle->mEnvironmentHandle);
         
         delete handle;
         handle = NULL;
         
         OsSysLog::add(FAC_ODBC, PRI_DEBUG,
                       "odbcDisconnect - disconnecting from database");
         ret = true;
      }
   }      
   else
   {
      OsSysLog::add(FAC_ODBC, PRI_ERR,
                    "odbcDisconnect: handle == NULL");
   }
   return ret;
}

bool odbcExecute(const OdbcHandle handle,
                 const char* sqlStatement)
{
   bool ret = false;
   
   if (handle)
   {
      SQLRETURN sqlRet;
      
      sqlRet = SQLExecDirect(handle->mStatementHandle,
                             (SQLCHAR*)sqlStatement,
                             SQL_NTS);
                             
      if (!SQL_SUCCEEDED(sqlRet))
      {
         OsSysLog::add(FAC_ODBC, PRI_ERR,
                       "odbcExecute: statement %s failed, error code %d",
                       sqlStatement, sqlRet);
      }
      else
      {
         OsSysLog::add(FAC_ODBC, PRI_DEBUG,
                       "odbcExecute: statement %s succeeded",
                       sqlStatement);         
         ret = true;
      }
   }
   else
   {
      OsSysLog::add(FAC_ODBC, PRI_ERR,
                    "odbcExecute: handle == NULL");
   }
   
   return ret;
}                  
              
int odbcResultColumns(const OdbcHandle handle)
{
   int ret = -1;
   
   if (handle)
   {
      SQLSMALLINT columns;
      SQLRETURN sqlRet = SQLNumResultCols(handle->mStatementHandle, &columns);
      
      if (!SQL_SUCCEEDED(sqlRet))
      {
         OsSysLog::add(FAC_ODBC, PRI_DEBUG,
                       "odbcResultColumns: SQLNumResultCols failed, "
                       "error code %d", sqlRet);         
      }
      else
      {
         ret = (int)columns;
         
         OsSysLog::add(FAC_ODBC, PRI_DEBUG,
                       "odbcResultColumns: SQLNumResultCols returned %d", ret);         
      }
   }
   else
   {
      OsSysLog::add(FAC_ODBC, PRI_ERR,
                    "odbcResultColumns: handle == NULL");            
   }
   
   return ret;
}

bool odbcGetNextRow(const OdbcHandle handle)
{
   bool ret = false;
   
   if (handle)
   {
      SQLRETURN sqlRet = SQLFetch(handle->mStatementHandle);
      
      if (!SQL_SUCCEEDED(sqlRet))
      {
         OsSysLog::add(FAC_ODBC, PRI_DEBUG,
                       "odbcGetNextRow: SQLFetch failed, error code %d",
                       sqlRet);         
      }
      else
      {
         OsSysLog::add(FAC_ODBC, PRI_DEBUG,
                       "odbcGetNextRow: SQLFetch succeeded");         
         ret = true;
      }
   }
   else
   {
      OsSysLog::add(FAC_ODBC, PRI_ERR,
                    "odbcGetNextRow: handle == NULL");      
   }
   
   return ret;
}
              
bool odbcGetColumnStringData(const OdbcHandle handle,
                             int columnIndex,
                             char* data,
                             int dataSize)
{
   bool ret = false;
   
   if (handle)
   {
      SQLLEN indicator;
      SQLRETURN sqlRet = SQLGetData(handle->mStatementHandle,
                                    (SQLUSMALLINT)columnIndex,
                                    SQL_C_CHAR,
                                    (SQLCHAR*)data,
                                    dataSize,
                                    &indicator);
      if (!SQL_SUCCEEDED(sqlRet))
      {
         OsSysLog::add(FAC_ODBC, PRI_WARNING,
                       "odbcGetColumnStringData: SQLGetData on column %d "
                       "failed, error code %d",
                       columnIndex, sqlRet);              
      }                           
      else
      {
         OsSysLog::add(FAC_ODBC, PRI_DEBUG,
                       "odbcGetColumnStringData: SQLGetData on column %d "
                       "returned %s",
                       columnIndex,
                       data);
         ret = true;
      }         
   }
   else
   {
      OsSysLog::add(FAC_ODBC, PRI_ERR,
                    "odbcGetColumnStringData: handle == NULL");            
   }
   
   return ret;
}

bool odbcClearResultSet(const OdbcHandle handle)
{
   bool ret = false;
   
   if (handle)
   {
      SQLRETURN sqlRet = SQLFreeStmt(handle->mStatementHandle,
                                     SQL_CLOSE);
                                     
      if (!SQL_SUCCEEDED(sqlRet))
      {
         OsSysLog::add(FAC_ODBC, PRI_WARNING,
                       "odbcClearResultSet: SQLFreeStmt failed, "
                       "error code %d", sqlRet);          
      }
      else
      {
         OsSysLog::add(FAC_ODBC, PRI_DEBUG,
                       "odbcClearResultSet: SQLFreeStmt succeeded");
         ret = true;              
      }
   }
   else
   {
      OsSysLog::add(FAC_ODBC, PRI_ERR,
                    "odbcClearResultSet: handle == NULL");          
   }
   return ret;
}
