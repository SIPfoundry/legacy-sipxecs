//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$

// Author: Scott Zuk
//         szuk AT telusplanet DOT net
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <os/OsTimerTask.h>
#include <os/OsProcess.h>
#include <os/OsStunAgentTask.h>
#include <os/OsSysLog.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipLineMgr.h>
#include <net/SipRefreshMgr.h>

#define SHUTDOWN_TEST_ITERATIONS 3

/**
 * Unittest for SipUserAgent
 */
class SipUserAgentTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipUserAgentTest);
      CPPUNIT_TEST(testShutdownBlocking);
      CPPUNIT_TEST(testShutdownNonBlocking);
      CPPUNIT_TEST(testSupportedAndRequiredFields);
      CPPUNIT_TEST_SUITE_END();

public:

   // OsProcess doesn't provide any thread info so this method returns
   // the number of threads running under the process given by PID.
   // FIXME: Only implemented for linux, always returns 1 otherwise.
   int getNumThreads( PID myPID )
   {
       int numThreads = 1;

#ifdef __linux__
       // /proc parsing stolen from OsProcessIteratorLinux.cpp
       OsStatus retval = OS_FAILED;
       char pidString[PID_STR_LEN];
       snprintf(pidString, PID_STR_LEN, "%ld", (long)myPID);

       OsPath fullProcName = "/proc/";
       fullProcName += pidString;
       fullProcName += "/status";
       OsFileLinux procFile(fullProcName);
       if (procFile.open(OsFile::READ_ONLY) == OS_SUCCESS)
       {
           size_t len = 5000; //since the length is always 0 for these files, lets try to read 5k
           char *buffer = new char[len+1];
           if (buffer)
           {
               size_t bytesRead;
               procFile.read((void *)buffer,len,bytesRead);

               if (bytesRead)
               {
                   procFile.close();
                   //null-terminate the string
                   buffer[bytesRead] = 0;
                   //now parse the info we need
                   char *ptr = strtok(buffer,"\n");
                   while(ptr)
                   {
                       if (memcmp(ptr,"Threads:",8) == 0)
                       {
                           numThreads = atoi(ptr+8);
                       }

                       ptr = strtok(NULL,"\n");
                   }

                   //say we are successful
                   retval = OS_SUCCESS;
               }
               else
                   osPrintf("Couldn't read bytes in readProcFile\n");

               delete [] buffer;
           }

           procFile.close();
       }
#endif
       return numThreads;
   }

   void testShutdownBlocking()
   {
      pid_t myPID = OsProcess::getCurrentPID();
      int startingThreads = getNumThreads(myPID);

      // Simple invite message from siptest/src/siptest/invite.txt
      const char* SimpleMessage =
          "INVITE sip:1@192.168.0.6 SIP/2.0\r\n"
          "Route: <sip:foo@192.168.0.4:5064;lr>\r\n"
          "From: <sip:888@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 8\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

      for(int i = 0; i < SHUTDOWN_TEST_ITERATIONS; ++i)
      {
         {
            SipLineMgr    lineMgr;
            SipRefreshMgr refreshMgr;

            lineMgr.StartLineMgr();
            lineMgr.initializeRefreshMgr( &refreshMgr );

            SipUserAgent sipUA( 5090
                               ,5090
                               ,5091
                               ,NULL     // default publicAddress
                               ,NULL     // default defaultUser
                               ,"127.0.0.1"     // default defaultSipAddress
                               ,NULL     // default sipProxyServers
                               ,NULL     // default sipDirectoryServers
                               ,NULL     // default sipRegistryServers
                               ,NULL     // default authenicateRealm
                               ,NULL     // default authenticateDb
                               ,NULL     // default authorizeUserIds
                               ,NULL     // default authorizePasswords
                               ,&lineMgr
                               );

            sipUA.start();
            refreshMgr.init(&sipUA);

            sipUA.send(testMsg);

            // Wait long enough for some stack timeouts/retansmits to occur
            OsTask::delay(10000); // 10 seconds

            // Shut down the tasks in reverse order.
            refreshMgr.requestShutdown();
            sipUA.shutdown(TRUE);
            lineMgr.requestShutdown();

            KNOWN_BUG("INTERMITTENT failures", "XX-6383");
            CPPUNIT_ASSERT(sipUA.isShutdownDone());

            OsTimerTask::destroyTimerTask();
            OsStunAgentTask::releaseInstance();
         }

         // Test to see that all the threads created by the above operations
         // get properly shut down.
         int numThreads = getNumThreads(myPID);

         OsSysLog::add(FAC_SIP, PRI_NOTICE, "SipUserAgentTest::testShutdownBlocking "
                       "numThreads=%d startingThreads=%d",
                       numThreads, startingThreads);

         KNOWN_BUG("XECS-48", "Some threads are not cleaned up?");
         CPPUNIT_ASSERT(numThreads <= startingThreads);
      }
   };

   void testShutdownNonBlocking()
   {
      pid_t myPID = OsProcess::getCurrentPID();
      int startingThreads = getNumThreads(myPID);

      // Simple invite message from siptest/src/siptest/invite.txt
      const char* SimpleMessage =
          "INVITE sip:1@192.168.0.6 SIP/2.0\r\n"
          "Route: <sip:foo@192.168.0.4:5064;lr>\r\n"
          "From: <sip:888@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 8\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

      for(int i = 0; i < SHUTDOWN_TEST_ITERATIONS; ++i)
      {
         {
            SipLineMgr    lineMgr;
            SipRefreshMgr refreshMgr;

            lineMgr.StartLineMgr();
            lineMgr.initializeRefreshMgr( &refreshMgr );

            SipUserAgent sipUA( 5090
                               ,5090
                               ,5091
                               ,NULL     // default publicAddress
                               ,NULL     // default defaultUser
                               ,"127.0.0.1"     // default defaultSipAddress
                               ,NULL     // default sipProxyServers
                               ,NULL     // default sipDirectoryServers
                               ,NULL     // default sipRegistryServers
                               ,NULL     // default authenicateRealm
                               ,NULL     // default authenticateDb
                               ,NULL     // default authorizeUserIds
                               ,NULL     // default authorizePasswords
                               ,&lineMgr
                               );

            sipUA.start();
            refreshMgr.init(&sipUA);

            sipUA.send(testMsg);

            // Wait long enough for some stack timeouts/retansmits to occur
            OsTask::delay(10000); // 10 seconds

            sipUA.shutdown(FALSE);
            lineMgr.requestShutdown();
            refreshMgr.requestShutdown();

            while(!sipUA.isShutdownDone())
            {
            }
            CPPUNIT_ASSERT(sipUA.isShutdownDone());

            OsTimerTask::destroyTimerTask();
            OsStunAgentTask::releaseInstance();
         }

         // Test to see that all the threads created by the above operations
         // get properly shut down.
         int numThreads = getNumThreads(myPID);

         OsSysLog::add(FAC_SIP, PRI_NOTICE, "SipUserAgentTest::testShutdownBlocking "
                       "numThreads=%d startingThreads=%d",
                       numThreads, startingThreads);

         KNOWN_BUG("XECS-48", "Some threads are not cleaned up?");
         CPPUNIT_ASSERT(numThreads <= startingThreads);
      }
   };

   void testSupportedAndRequiredFields()
   {
      SipUserAgent sipUA( 5090, 5090, 5091,
                          NULL, NULL,   // default publicAddress and defaultUser
                         "127.0.0.1" );

      // Supported
      CPPUNIT_ASSERT( ! sipUA.isExtensionAllowed( "nope" ) );

      UtlString supported( "supported-1" );
      sipUA.allowExtension( supported );

      UtlString tmp;
      sipUA.getSupportedExtensions( tmp );
      CPPUNIT_ASSERT( supported == tmp );
      supported.toUpper();
      CPPUNIT_ASSERT( sipUA.isExtensionAllowed( supported ) );
      CPPUNIT_ASSERT( ! sipUA.isExtensionAllowed( "nope" ) );


      // Required
      CPPUNIT_ASSERT( ! sipUA.isExtensionRequired( "nope" ) );

      UtlString required( "required-1, required-2, required-3" );
      UtlString copy_required( required );
      copy_required += ',';
      ssize_t prev = 0;
      ssize_t index = copy_required.index( ',', prev );
      while( UTL_NOT_FOUND != index )
      {
         UtlString field = copy_required( prev, index - prev );
         field.strip( UtlString::both );
         sipUA.requireExtension( field );
         prev = index + 1;
         index = copy_required.index( ',', prev );
      }

      sipUA.getRequiredExtensions( tmp );
      CPPUNIT_ASSERT( required == tmp );
      index = copy_required.index( ',', prev );
      while( UTL_NOT_FOUND != index )
      {
         UtlString field = copy_required( prev, index - prev );
         field.strip( UtlString::both );
         field.toUpper();
         CPPUNIT_ASSERT( sipUA.isExtensionRequired( field ) );
         prev = index + 1;
         index = copy_required.index( ',', prev );
      }
      CPPUNIT_ASSERT( ! sipUA.isExtensionRequired( "nope" ) );
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipUserAgentTest);
