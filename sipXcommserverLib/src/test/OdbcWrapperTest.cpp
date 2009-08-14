//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "odbc/OdbcWrapper.h"

#ifdef TESTDATABASE
#define DATABASE_NAME   TESTDATABASE
#else
#define DATABASE_NAME   "SIPXCDR-TEST"
#endif

// DEFINES
#define ODBC_LOGGING
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class OdbcWrapperTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(OdbcWrapperTest);
   CPPUNIT_TEST(testOdbcConnect);
   CPPUNIT_TEST(testOdbcExecute);
   CPPUNIT_TEST(testOdbcValidateObserverData);
   CPPUNIT_TEST(testOdbcValidateEventData);
   CPPUNIT_TEST_SUITE_END();

public:

   void setUp()
      {
#ifdef ODBC_LOGGING
         OsSysLog::initialize(0, "odbc");
         OsSysLog::setOutputFile(0, "odbcTest.log");
         OsSysLog::setLoggingPriority(PRI_DEBUG);
#endif
      }

   void tearDown()
      {
         OdbcHandle handle = NULL;

         CPPUNIT_ASSERT((handle=odbcConnect(DATABASE_NAME,
                                            "localhost",
                                            POSTGRESQL_USER,
                                            "{PostgreSQL}"))!=NULL);
         if (handle)
         {
            char sqlStatement[256];
            // Clear tables
            sprintf(sqlStatement, "DELETE FROM call_state_events *;");
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));
            sprintf(sqlStatement, "DELETE FROM observer_state_events *;");
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));
         }
#ifdef ODBC_LOGGING
         OsSysLog::flush();
#endif
      }

   void databaseWrite(OdbcHandle handle)
      {
         if (handle)
         {
            char sqlStatement[256];
            // Clear tables
            sprintf(sqlStatement, "DELETE FROM call_state_events *;");
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));
            sprintf(sqlStatement, "DELETE FROM observer_state_events *;");
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));

            sprintf(sqlStatement,
                    "INSERT INTO observer_state_events VALUES (DEFAULT,"
                    "\'10.1.20.3:5060\',"
                    "0,"
                    "timestamp \'2006-03-10 12:59:00.666\',"
                    "101,"
                    "\'AuthProxyCseObserver\');");

            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));

            sprintf(sqlStatement,
                    "INSERT INTO call_state_events VALUES (DEFAULT,"
                    "\'10.1.20.3:5060\',"
                    "1,"
                    "timestamp \'2006-03-10 13:00:00.123\',"
                    "\'R\',"
                    "12,"
                    "\'call-111111\',"
                    "\'12345\',"
                    "\'67890\',"
                    "\'sip:153@example.com\',"
                    "\'sip:202@example.com\',"
                    "\'10.1.1.71\',"
                    "\'refer-to\',"
                    "\'referred-by\',"
                    "0,"
                    "\'No Reason\',"
                    "\'\');");

            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));

            sprintf(sqlStatement,
                    "INSERT INTO call_state_events VALUES (DEFAULT,"
                    "\'10.1.20.3:5060\',"
                    "2,"
                    "timestamp \'2006-03-10 13:00:10.573\',"
                    "\'S\',"
                    "13,"
                    "\'call-111112\',"
                    "\'54321\',"
                    "\'09876\',"
                    "\'sip:156@example.com\',"
                    "\'sip:215@example.com\',"
                    "\'10.1.20.71\',"
                    "\'refer-to-2\',"
                    "\'referred-by-2\',"
                    "0,"
                    "\'No Reason-2\',"
                    "\'\');");

            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));
         }
      }

   void testOdbcConnect()
      {
         OdbcHandle handle = NULL;

         CPPUNIT_ASSERT((handle=odbcConnect(DATABASE_NAME,
                                            "localhost",
                                            POSTGRESQL_USER,
                                            "{PostgreSQL}"))!=NULL);

         if (handle)
         {
            CPPUNIT_ASSERT(odbcDisconnect(handle));
         }
      }

   void testOdbcExecute()
      {
         OdbcHandle handle = NULL;

         CPPUNIT_ASSERT((handle=odbcConnect(DATABASE_NAME,
                                            "localhost",
                                            POSTGRESQL_USER,
                                            "{PostgreSQL}"))!=NULL);
         if (handle)
         {
            databaseWrite(handle);

            CPPUNIT_ASSERT(odbcDisconnect(handle));
         }
      }

   void testOdbcValidateObserverData()
      {
         OdbcHandle handle = NULL;
         char sqlStatement[256];

         CPPUNIT_ASSERT((handle=odbcConnect(DATABASE_NAME,
                                            "localhost",
                                            POSTGRESQL_USER,
                                            "{PostgreSQL}"))!=NULL);

         if (handle)
         {
            databaseWrite(handle);
            int cols;

            // Get data from call_state_observer_events
            sprintf(sqlStatement, "SELECT * FROM observer_state_events;");
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));

            CPPUNIT_ASSERT((cols=odbcResultColumns(handle)) == 6);

            CPPUNIT_ASSERT(odbcGetNextRow(handle));

            char buffer[256];

            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 2, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "10.1.20.3:5060") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 3, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "0") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 4, buffer, 256));
            // When returning a timestamp as string data the fractional component
            // is truncated - this is expected behavior
            CPPUNIT_ASSERT(strcmp(buffer, "2006-03-10 12:59:00") == 0);

            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 5, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "101") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 6, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "AuthProxyCseObserver") == 0);

            CPPUNIT_ASSERT(odbcDisconnect(handle));
         }
      }

   void testOdbcValidateEventData()
      {
         OdbcHandle handle = NULL;
         char sqlStatement[256];

         CPPUNIT_ASSERT((handle=odbcConnect(DATABASE_NAME,
                                            "localhost",
                                            POSTGRESQL_USER,
                                            "{PostgreSQL}"))!=NULL);

         if (handle)
         {
            databaseWrite(handle);

            int cols;

            // Get data from call_state_events
            sprintf(sqlStatement, "SELECT * FROM call_state_events;");
            CPPUNIT_ASSERT(odbcExecute(handle, sqlStatement));

            CPPUNIT_ASSERT((cols=odbcResultColumns(handle)) == 17);

            CPPUNIT_ASSERT(odbcGetNextRow(handle));

            char buffer[256];

            // First record
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 2, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "10.1.20.3:5060") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 3, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "1") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 4, buffer, 256));
            // When returning a timestamp as string data the fractional component
            // is truncated - this is expected behavior
            CPPUNIT_ASSERT(strcmp(buffer, "2006-03-10 13:00:00") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 5, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "R") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 7, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "call-111111") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 8, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "12345") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 9, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "67890") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 10, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "sip:153@example.com") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 11, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "sip:202@example.com") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 12, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "10.1.1.71") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 13, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "refer-to") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 14, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "referred-by") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 15, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "0") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 16, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "No Reason") == 0);

            CPPUNIT_ASSERT(odbcGetNextRow(handle));

            // Second record
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 2, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "10.1.20.3:5060") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 3, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "2") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 4, buffer, 256));
            // When returning a timestamp as string data the fractional component
            // is truncated - this is expected behavior
            CPPUNIT_ASSERT(strcmp(buffer, "2006-03-10 13:00:10") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 5, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "S") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 7, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "call-111112") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 8, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "54321") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 9, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "09876") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 10, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "sip:156@example.com") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 11, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "sip:215@example.com") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 12, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "10.1.20.71") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 13, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "refer-to-2") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 14, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "referred-by-2") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 15, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "0") == 0);
            CPPUNIT_ASSERT(odbcGetColumnStringData(handle, 16, buffer, 256));
            CPPUNIT_ASSERT(strcmp(buffer, "No Reason-2") == 0);

            // End of rows
            CPPUNIT_ASSERT(!odbcGetNextRow(handle));

            CPPUNIT_ASSERT(odbcDisconnect(handle));
         }
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OdbcWrapperTest);
