//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _ODBCWRAPPER_H_
#define _ODBCWRAPPER_H_

// SYSTEM INCLUDES
#include <stdio.h>
#include <sql.h>
#include <sqlext.h>

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

struct OdbcControlStruct {
   SQLHENV mEnvironmentHandle;
   SQLHDBC mConnectionHandle;
   SQLHSTMT mStatementHandle;
};

typedef OdbcControlStruct* OdbcHandle;

/// Tries to establish DSN-less connection to the database
/*! \param dbname (in) - name of the data source
 * \param servername (in) - server name
 * \param username (in) - name of the user
 * \param password (in) - password to connect to data source
 * \param driver (in) - driver name used to connect
 * \returns  a handle representing the connection
 */
OdbcHandle odbcConnect(const char* dbname,
                       const char* servername,
                       const char* username,
                       const char* driver,
                       const char* password=NULL);

/// Disconnects from data source and frees all resources
/*! \param handle (in) - handle returned by odbcConnect
 * \returns  true if disconnected sucessfully, otherwise false
 */
bool odbcDisconnect(OdbcHandle &handle);

/// Executes an SQL statement
/*! \param handle (in) - handle returned by odbcConnect
 * \param sqlStatement (in) - pointer to string containing the SQL statement
 * \returns  true if connected to data source otherwise false
 */
bool odbcExecute(const OdbcHandle handle,
                 const char* sqlStatement);

/// Returns the number of columns in a result set
/*! \param handle (in) - handle returned by odbcConnect
 * \returns   number of columns in result set
 */
int odbcResultColumns(const OdbcHandle handle);

/// Moves to the next row in the result set, reset by new result set
/*! \param handle (in) - handle returned by odbcConnect
 * \returns   true while there is a next row to move to
 */
bool odbcGetNextRow(const OdbcHandle handle);

/// Returns column string data from current row
/*! \param handle (in) - handle returned by odbcConnect
 * \param columnIndex (in) - column index in row
 * \param data (out) - data buffer to receive string data
 * \param dataSize (in) - size of buffer
 * \returns   true while there is a next row to move to
 */
bool odbcGetColumnStringData(const OdbcHandle handle,
                             int columnIndex,
                             char* data,
                             int dataSize);

// Clears the result set and enables further retrieval
/*! \param handle (in) - handle returned by odbcConnect
 * \returns   true if result set could be cleared
 */
bool odbcClearResultSet(const OdbcHandle handle);

#endif // _ODBCWRAPPER_H_
