//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sys/types.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <resolv.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>

#include "net/SipSrvLookup.h"
#include "os/OsSocket.h"

// Defines
//#define TEST_PRINT

// Expects g++ to have defined TESTDIR as the name of the directory
// containing sipXtackLib/src/test, so SipSrvLookupTest.cpp can find
// its auxiliary files.

// Path to the named executable is NAMED_PROGRAM, which is supplied by
// a -D to the compiler.  If it is not supplied, the tests that require
// it will not be built.
// Port for the test named to listen on.
#define NAMED_PORT 13253

// Forward references.

// Get a printable representation of a protocol value.
const char* printable_proto(OsSocket::IpProtocolSocketType type);

/**
 * Unit test for SipSrvLookup
 */
class SipSrvLookupTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipSrvLookupTest);
   CPPUNIT_TEST(lookup);
   CPPUNIT_TEST_SUITE_END();

public:

   void lookup()
   {
#ifdef NAMED_PROGRAM
      FILE* f;
      // Buffer in which to construct result string.
      char result_string[1024];

      // Check that named is executable.
      struct stat statbuf;
      CPPUNIT_ASSERT(stat(NAMED_PROGRAM, &statbuf) == 0);
      CPPUNIT_ASSERT(S_ISREG(statbuf.st_mode));
      CPPUNIT_ASSERT((statbuf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0);

      // Locate temporary directory.
      const char* temp_dir = getenv("TMPDIR");
      if (temp_dir == NULL || temp_dir[0] == '\0')
      {
         temp_dir = "/tmp";
      }
      pid_t pid = getpid();

      // Set up named configuration file.
      char config_file[100];
      sprintf(config_file, "%s/%ld.conf", temp_dir, (long)pid);
      f = fopen(config_file, "w");
      CPPUNIT_ASSERT(f != NULL);
      fprintf(f, "zone \".\" IN {\n");
      fprintf(f, "\ttype master;\n");
      fprintf(f, "\tfile \"SipSrvLookupTest.named.zone\";\n");
      fprintf(f, "\tallow-update { none; };\n");
      fprintf(f, "};\n");
      fprintf(f, "options {\n");
      fprintf(f, "\tdirectory \"%s\";\n", TESTDIR);
      fprintf(f, "\tpid-file \"%s/%ld.pid\";\n", temp_dir, (long)pid);
      fprintf(f, "};\n");
      fprintf(f, "controls { };\n");
      fclose(f);

      // Set up test copy of named.
      // Set up macros for the argument list to execl(), and how to print it.
      char b[10];
      sprintf(b, "%d", NAMED_PORT);
      // Remember that the first argument is the executable to run, and
      // the second one is $0, which may or may not be the same.
      // The -c gives the configuration file name, the -p gives the port to
      // listen on, and the -f prevents named from trying to background itself,
      // that is, to fork again, which would make the real named process number
      // different from named_pid.
      #define EXECL_ARGS \
		NAMED_PROGRAM, NAMED_PROGRAM, "-c", config_file, "-p", b, "-f"
      // A format string that can print the values listed in EXCL_ARGS.
      #define EXECL_ARGS_FORMAT "%s %s %s %s %s %s %s"
      pid_t named_pid = fork();
      CPPUNIT_ASSERT(named_pid >= 0);
      if (named_pid == 0) {
         // This is the child process.
         execl(EXECL_ARGS, NULL);
         // Shouldn't get here.
         fprintf(stderr, "execl(" EXECL_ARGS_FORMAT ") returned error %d\n",
                 EXECL_ARGS, errno);
         CPPUNIT_ASSERT(FALSE);
         exit(1);
      }
      // This is the parent process.
#     ifdef TEST_PRINT
      printf("Starting bind with execl(" EXECL_ARGS_FORMAT "), process %ld\n",
             EXECL_ARGS, (long)named_pid);
#     endif

      // Wait a bit so bind can start up.
      sleep(10);

      // Set the nameserver address to our private bind
      SipSrvLookup::set_nameserver_address("127.0.0.1", NAMED_PORT);

#     ifdef TEST_PRINT
      printf("_res.nscount = %d\n", _res.nscount);
      for (int i = 0; i < _res.nscount; i++)
      {
         printf("_res.nsaddr_list[%d] = %s:%d\n",
                i, inet_ntoa(_res.nsaddr_list[i].sin_addr),
                ntohs(_res.nsaddr_list[i].sin_port));
      }
#     endif /* TEST_PRINT */

      // List of tests to execute.
      struct test {
         // Arguments to SipSrvLookup::servers:
         const char* name;
         const char* service;
         OsSocket::IpProtocolSocketType type;
         int port;
         // List of option values to set for this test, or NULL.
         // If not NULL, is a pointer to a list of int's, which are:
         //    option1, value1, option2, value2, ..., OptionCodeNone.
         int* options;
         // String encoding the expected results, in the format:
         // "IP:port,weight,score,priority,proto\n" for each result.
         // (Score is rounded to 3 digits.)
         // srand() is called before each test to make rand() reproducible.
         const char* expected;
      };

      // Option value setting lists.
      int options_ignore_SRV[] =
         { SipSrvLookup::OptionCodeIgnoreSRV, 1,
           SipSrvLookup::OptionCodeNone };
      int options_no_TCP[] =
         { SipSrvLookup::OptionCodeNoDefaultTCP, 1,
           SipSrvLookup::OptionCodeNone };

      struct test tests[] = {

         // "sip" service

         // Numeric IP address.
         { "1.1.0.1", "sip", OsSocket::UNKNOWN, -1, NULL,
           "1.1.0.1:5060,0,1000.000,0,UDP\n"
           "1.1.0.1:5060,0,1000.000,0,TCP\n" },
         // Numeric IP address with port.
         { "1.1.0.1", "sip", OsSocket::UNKNOWN, 101, NULL,
           "1.1.0.1:101,0,1000.000,0,UDP\n"
           "1.1.0.1:101,0,1000.000,0,TCP\n" },

         // Name with A record.
         { "test2", "sip", OsSocket::UNKNOWN, -1, NULL,
           "1.2.1.0:5060,0,1000.000,0,UDP\n"
           "1.2.1.0:5060,0,1000.000,0,TCP\n" },
         // Name with A record with port.
         { "test2", "sip", OsSocket::UNKNOWN, 102, NULL,
           "1.2.1.0:102,0,1000.000,0,UDP\n"
           "1.2.1.0:102,0,1000.000,0,TCP\n" },
         // Name with two A records.
         { "test3", "sip", OsSocket::UNKNOWN, -1, NULL,
           "1.3.1.0:5060,0,1000.000,0,UDP\n"
           "1.3.1.1:5060,0,1000.000,0,UDP\n"
           "1.3.1.0:5060,0,1000.000,0,TCP\n"
           "1.3.1.1:5060,0,1000.000,0,TCP\n" },
         // Name with two A records with port.
         { "test3", "sip", OsSocket::UNKNOWN, 103, NULL,
           "1.3.1.0:103,0,1000.000,0,UDP\n"
           "1.3.1.1:103,0,1000.000,0,UDP\n"
           "1.3.1.0:103,0,1000.000,0,TCP\n"
           "1.3.1.1:103,0,1000.000,0,TCP\n" },

         // Name with UDP SRV record.
         { "test4", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.4.0:5060,1,0.174,1,UDP\n" },
         // Name with UDP SRV record with nonstandard port.
         { "test5", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.5.0:5999,1,0.174,1,UDP\n" },
         // Name with two UDP SRV records.
         { "test6", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.6.0:5066,1,0.174,1,UDP\n"
           "2.1.6.1:5067,1,0.930,1,UDP\n" },
         // Name with UDP SRV record but port specified (which suppresses
         // the UDP SRV record).
         { "test6", "sip", OsSocket::UNKNOWN, 106, NULL,
           "1.6.1.0:106,0,1000.000,0,UDP\n"
           "1.6.1.0:106,0,1000.000,0,TCP\n" },
         // Name with UDP SRV record but no A record for the target.
         { "test10", "sip", OsSocket::UNKNOWN, 106, NULL,
           "1.10.1.0:106,0,1000.000,0,UDP\n"
           "1.10.1.0:106,0,1000.000,0,TCP\n" },

         // Name with TCP SRV record.
         { "test7", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.4.0:5060,1,0.174,1,TCP\n" },
         // Name with TCP SRV record with nonstandard port.
         { "test8", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.5.0:5999,1,0.174,1,TCP\n" },
         // Name with two TCP SRV records.
         { "test9", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.6.0:5066,1,0.174,1,TCP\n"
           "2.1.6.1:5067,1,0.930,1,TCP\n" },
         // Name with TCP SRV, but no A record for the target.
         { "test11", "sip", OsSocket::UNKNOWN, -1, NULL,
           "1.11.1.0:5060,0,1000.000,0,UDP\n"
           "1.11.1.0:5060,0,1000.000,0,TCP\n" },
         // Name with TCP SRV record but port specified (which suppresses
         // the TCP SRV record).
         { "test9", "sip", OsSocket::UNKNOWN, 106, NULL,
           "1.9.1.0:106,0,1000.000,0,UDP\n"
           "1.9.1.0:106,0,1000.000,0,TCP\n" },

         // Name with UDP and TCP SRV records.
         { "test12", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.6.0:666,1,0.174,1,UDP\n"
           "2.1.6.1:667,1,0.930,1,TCP\n" },

         // "sip" service, with UDP transport specified.

         // Numeric IP address.
         { "1.1.0.1", "sip", OsSocket::UDP, -1, NULL,
           "1.1.0.1:5060,0,1000.000,0,UDP\n" },
         // Numeric IP address with port.
         { "1.1.0.1", "sip", OsSocket::UDP, 101, NULL,
           "1.1.0.1:101,0,1000.000,0,UDP\n" },

         // Name with A record.
         { "test2", "sip", OsSocket::UDP, -1, NULL,
           "1.2.1.0:5060,0,1000.000,0,UDP\n" },
         // Name with A record with port.
         { "test2", "sip", OsSocket::UDP, 102, NULL,
           "1.2.1.0:102,0,1000.000,0,UDP\n" },
         // Name with two A records.
         { "test3", "sip", OsSocket::UDP, -1, NULL,
           "1.3.1.0:5060,0,1000.000,0,UDP\n"
           "1.3.1.1:5060,0,1000.000,0,UDP\n" },
         // Name with two A records with port.
         { "test3", "sip", OsSocket::UDP, 103, NULL,
           "1.3.1.0:103,0,1000.000,0,UDP\n"
           "1.3.1.1:103,0,1000.000,0,UDP\n" },

         // Name with UDP SRV record.
         { "test4", "sip", OsSocket::UDP, -1, NULL,
           "2.1.4.0:5060,1,0.174,1,UDP\n" },
         // Name with UDP SRV record with nonstandard port.
         { "test5", "sip", OsSocket::UDP, -1, NULL,
           "2.1.5.0:5999,1,0.174,1,UDP\n" },
         // Name with two UDP SRV records.
         { "test6", "sip", OsSocket::UDP, -1, NULL,
           "2.1.6.0:5066,1,0.174,1,UDP\n"
           "2.1.6.1:5067,1,0.930,1,UDP\n" },
         // Name with UDP SRV record but port specified (which suppresses
         // the UDP SRV record).
         { "test6", "sip", OsSocket::UDP, 106, NULL,
           "1.6.1.0:106,0,1000.000,0,UDP\n" },
         // Name with UDP SRV record but no A record for the target.
         { "test10", "sip", OsSocket::UDP, 106, NULL,
           "1.10.1.0:106,0,1000.000,0,UDP\n" },

         // Name with TCP SRV record.
         { "test7", "sip", OsSocket::UDP, -1, NULL,
           "" },
         // Name with TCP SRV record with nonstandard port.
         { "test8", "sip", OsSocket::UDP, -1, NULL,
           "" },
         // Name with two TCP SRV records.
         { "test9", "sip", OsSocket::UDP, -1, NULL,
           "1.9.1.0:5060,0,1000.000,0,UDP\n" },
         // Name with TCP SRV, but no A record for the target.
         { "test11", "sip", OsSocket::UDP, -1, NULL,
           "1.11.1.0:5060,0,1000.000,0,UDP\n" },
         // Name with TCP SRV record but port specified (which suppresses
         // the TCP SRV record).
         { "test9", "sip", OsSocket::UDP, 106, NULL,
           "1.9.1.0:106,0,1000.000,0,UDP\n" },

         // Name with UDP and TCP SRV records.
         { "test12", "sip", OsSocket::UDP, -1, NULL,
           "2.1.6.0:666,1,0.174,1,UDP\n" },

         // "sip" service, with TCP transport specified.

         // Numeric IP address.
         { "1.1.0.1", "sip", OsSocket::TCP, -1, NULL,
           "1.1.0.1:5060,0,1000.000,0,TCP\n" },
         // Numeric IP address with port.
         { "1.1.0.1", "sip", OsSocket::TCP, 101, NULL,
           "1.1.0.1:101,0,1000.000,0,TCP\n" },

         // Name with A record.
         { "test2", "sip", OsSocket::TCP, -1, NULL,
           "1.2.1.0:5060,0,1000.000,0,TCP\n" },
         // Name with A record with port.
         { "test2", "sip", OsSocket::TCP, 102, NULL,
           "1.2.1.0:102,0,1000.000,0,TCP\n" },
         // Name with two A records.
         { "test3", "sip", OsSocket::TCP, -1, NULL,
           "1.3.1.0:5060,0,1000.000,0,TCP\n"
           "1.3.1.1:5060,0,1000.000,0,TCP\n" },
         // Name with two A records with port.
         { "test3", "sip", OsSocket::TCP, 103, NULL,
           "1.3.1.0:103,0,1000.000,0,TCP\n"
           "1.3.1.1:103,0,1000.000,0,TCP\n" },

         // Name with UDP SRV record.
         { "test4", "sip", OsSocket::TCP, -1, NULL,
           "" },
         // Name with UDP SRV record with nonstandard port.
         { "test5", "sip", OsSocket::TCP, -1, NULL,
           "" },
         // Name with two UDP SRV records.
         { "test6", "sip", OsSocket::TCP, -1, NULL,
           "1.6.1.0:5060,0,1000.000,0,TCP\n" },
         // Name with UDP SRV record but port specified (which suppresses
         // the UDP SRV record).
         { "test6", "sip", OsSocket::TCP, 106, NULL,
           "1.6.1.0:106,0,1000.000,0,TCP\n" },
         // Name with UDP SRV record but no A record for the target.
         { "test10", "sip", OsSocket::TCP, 106, NULL,
           "1.10.1.0:106,0,1000.000,0,TCP\n" },

         // Name with TCP SRV record.
         { "test7", "sip", OsSocket::TCP, -1, NULL,
           "2.1.4.0:5060,1,0.174,1,TCP\n" },
         // Name with TCP SRV record with nonstandard port.
         { "test8", "sip", OsSocket::TCP, -1, NULL,
           "2.1.5.0:5999,1,0.174,1,TCP\n" },
         // Name with two TCP SRV records.
         { "test9", "sip", OsSocket::TCP, -1, NULL,
           "2.1.6.0:5066,1,0.174,1,TCP\n"
           "2.1.6.1:5067,1,0.930,1,TCP\n" },
         // Name with TCP SRV, but no A record for the target.
         { "test11", "sip", OsSocket::TCP, -1, NULL,
           "1.11.1.0:5060,0,1000.000,0,TCP\n" },
         // Name with TCP SRV record but port specified (which suppresses
         // the TCP SRV record).
         { "test9", "sip", OsSocket::TCP, 106, NULL,
           "1.9.1.0:106,0,1000.000,0,TCP\n" },

         // Name with UDP and TCP SRV records.
         { "test12", "sip", OsSocket::TCP, -1, NULL,
           "2.1.6.1:667,1,0.174,1,TCP\n" },

         // "sip" service, with TLS transport specified.

         // Numeric IP address.
         { "1.1.0.1", "sip", OsSocket::SSL_SOCKET, -1, NULL,
           "1.1.0.1:5061,0,1000.000,0,TLS\n" },
         // Numeric IP address with port.
         { "1.1.0.1", "sip", OsSocket::SSL_SOCKET, 101, NULL,
           "1.1.0.1:101,0,1000.000,0,TLS\n" },
         // Name with A record.
         { "test2", "sip", OsSocket::SSL_SOCKET, -1, NULL,
           "1.2.1.0:5061,0,1000.000,0,TLS\n" },
         // Name with A record with port.
         { "test2", "sip", OsSocket::SSL_SOCKET, 102, NULL,
           "1.2.1.0:102,0,1000.000,0,TLS\n" },
         // Name with UDP SRV record.
         { "test4", "sip", OsSocket::SSL_SOCKET, -1, NULL,
           "" },
         // Name with UDP SRV record with nonstandard port.
         { "test5", "sip", OsSocket::SSL_SOCKET, -1, NULL,
           "" },
         // Name with TCP SRV record.
         { "test7", "sip", OsSocket::SSL_SOCKET, -1, NULL,
           "" },
         // Name with TCP SRV record with nonstandard port.
         { "test8", "sip", OsSocket::SSL_SOCKET, -1, NULL,
           "" },

         // Tests to check OptionCodeIgnoreSRV.

         // Name with UDP SRV record.
         { "test4", "sip", OsSocket::UNKNOWN, -1, options_ignore_SRV,
           "" },
         // Name with TCP SRV record.
         { "test7", "sip", OsSocket::UNKNOWN, -1, options_ignore_SRV,
           "" },
         // Name with two TCP SRV records.
         { "test9", "sip", OsSocket::UNKNOWN, -1, options_ignore_SRV,
           "1.9.1.0:5060,0,1000.000,0,UDP\n"
           "1.9.1.0:5060,0,1000.000,0,TCP\n" },

         // Test weighting and priority.

         { "test20", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.4.0:8000,1,0.174,1,UDP\n"
           "2.1.4.0:8001,1,0.930,2,UDP\n"
           "2.1.4.0:8002,1,0.244,3,UDP\n" },
         { "test21", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.4.0:8002,1,0.244,1,UDP\n"
           "2.1.4.0:8001,1,0.930,2,UDP\n"
           "2.1.4.0:8000,1,0.174,3,UDP\n" },
         { "test22", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.4.0:8000,1,0.930,1,UDP\n"
           "2.1.4.0:8001,1,0.174,2,TCP\n"
           "2.1.4.0:8002,1,0.244,3,TLS\n" },
         { "test23", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.4.0:8000,1,0.174,1,UDP\n"
           "2.1.4.0:8003,1,0.225,1,UDP\n"
           "2.1.4.0:8002,1,0.244,1,UDP\n"
           "2.1.4.0:8001,1,0.930,1,UDP\n" },
         { "test24", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.4.0:8003,2,0.113,1,UDP\n"
           "2.1.4.0:8002,2,0.122,1,UDP\n"
           "2.1.4.0:8000,1,0.174,1,UDP\n"
           "2.1.4.0:8001,1,0.930,1,UDP\n" },

         // Tests with CNAME records.

         // Chains of CNAME records to A records.
         { "c0.test25", "sip", OsSocket::UDP, -1, NULL,
           "1.25.1.0:5060,0,1000.000,0,UDP\n" },
         { "c1.test25", "sip", OsSocket::UDP, -1, NULL,
           "1.25.1.0:5060,0,1000.000,0,UDP\n" },
         { "c2.test25", "sip", OsSocket::UDP, -1, NULL,
           "1.25.1.0:5060,0,1000.000,0,UDP\n" },
         { "c3.test25", "sip", OsSocket::UDP, -1, NULL,
           "1.25.1.0:5060,0,1000.000,0,UDP\n" },
         { "c4.test25", "sip", OsSocket::UDP, -1, NULL,
           "1.25.1.0:5060,0,1000.000,0,UDP\n" },
         { "c5.test25", "sip", OsSocket::UDP, -1, NULL,
           "1.25.1.0:5060,0,1000.000,0,UDP\n" },
         // This lookup goes through 6 CNAMEs, whereas the configured limit
         // is 5.
         { "c6.test25", "sip", OsSocket::UDP, -1, NULL,
           "" },

         // SRV with target CNAMEs to A records.
         { "test26", "sip", OsSocket::UNKNOWN, -1, NULL,
           "1.25.1.0:5060,1,0.174,1,UDP\n" },
         { "test27", "sip", OsSocket::UNKNOWN, -1, NULL,
           "1.25.1.0:667,1,0.174,1,TCP\n"
           "1.25.1.0:666,1,0.930,1,UDP\n" },
         { "test27", "sip", OsSocket::TCP, -1, NULL,
           "1.25.1.0:667,1,0.174,1,TCP\n" },

         // Tests with OptionCodeNoDefaultTCP set.

         // Numeric IP address.
         { "1.1.0.1", "sip", OsSocket::UNKNOWN, -1, options_no_TCP,
           "1.1.0.1:5060,0,1000.000,0,UDP\n" },
         // Numeric IP address with port.
         { "1.1.0.1", "sip", OsSocket::UNKNOWN, 101, options_no_TCP,
           "1.1.0.1:101,0,1000.000,0,UDP\n" },
         // Name with A record.
         { "test2", "sip", OsSocket::UNKNOWN, -1, options_no_TCP,
           "1.2.1.0:5060,0,1000.000,0,UDP\n" },
         // Name with A record with port.
         { "test2", "sip", OsSocket::UNKNOWN, 102, options_no_TCP,
           "1.2.1.0:102,0,1000.000,0,UDP\n" },
         // Name with two A records.
         { "test3", "sip", OsSocket::UNKNOWN, -1, options_no_TCP,
           "1.3.1.0:5060,0,1000.000,0,UDP\n"
           "1.3.1.1:5060,0,1000.000,0,UDP\n" },
         // Name with two A records with port.
         { "test3", "sip", OsSocket::UNKNOWN, 103, options_no_TCP,
           "1.3.1.0:103,0,1000.000,0,UDP\n"
           "1.3.1.1:103,0,1000.000,0,UDP\n" },
         { "test6", "sip", OsSocket::UNKNOWN, 106, options_no_TCP,
           "1.6.1.0:106,0,1000.000,0,UDP\n" },
         // Name with UDP SRV record but no A record for the target.
         { "test10", "sip", OsSocket::UNKNOWN, 106, options_no_TCP,
           "1.10.1.0:106,0,1000.000,0,UDP\n" },
         { "test11", "sip", OsSocket::UNKNOWN, -1, options_no_TCP,
           "1.11.1.0:5060,0,1000.000,0,UDP\n" },
         // Name with TCP SRV record but port specified (which suppresses
         // the TCP SRV record).
         { "test9", "sip", OsSocket::UNKNOWN, 106, options_no_TCP,
           "1.9.1.0:106,0,1000.000,0,UDP\n" },
         // Name with two A records.
         { "test3", "sip", OsSocket::TCP, -1, options_no_TCP,
           "1.3.1.0:5060,0,1000.000,0,TCP\n"
           "1.3.1.1:5060,0,1000.000,0,TCP\n" },
         // Name with two A records with port.
         { "test3", "sip", OsSocket::TCP, 103, options_no_TCP,
           "1.3.1.0:103,0,1000.000,0,TCP\n"
           "1.3.1.1:103,0,1000.000,0,TCP\n" },

         // Check that OptionCodeNoDefaultTCP does not suppress TCP when
         // it is specified.
         // Numeric IP address.
         { "1.1.0.1", "sip", OsSocket::TCP, -1, options_no_TCP,
           "1.1.0.1:5060,0,1000.000,0,TCP\n" },
         // Numeric IP address with port.
         { "1.1.0.1", "sip", OsSocket::TCP, 101, options_no_TCP,
           "1.1.0.1:101,0,1000.000,0,TCP\n" },
         // Name with A record.
         { "test2", "sip", OsSocket::TCP, -1, options_no_TCP,
           "1.2.1.0:5060,0,1000.000,0,TCP\n" },
         // Name with A record with port.
         { "test2", "sip", OsSocket::TCP, 102, options_no_TCP,
           "1.2.1.0:102,0,1000.000,0,TCP\n" },

         // Tests to ensure that sorting on TCP vs. UDP does not override
         // priority or score.
         { "test28", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.4.0:8001,1,0.244,1,UDP\n"
           "2.1.4.0:8002,1,0.174,2,TCP\n"
           "2.1.4.0:8003,1,0.225,3,UDP\n"
           "2.1.4.0:8004,1,0.930,4,TCP\n"
           "" },
         { "test29", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.4.0:8001,1000,0.000,1,UDP\n"
           "2.1.4.0:8002,100,0.002,1,TCP\n"
           "2.1.4.0:8003,10,0.023,1,UDP\n"
           "2.1.4.0:8004,1,0.930,1,TCP\n"
           "" },

         // Test to see that our own name _does_ override weights
         { "test30", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.8.1:5060,1,0.244,1,TCP\n"
           "2.1.8.1:5060,1,0.225,1,UDP\n"
           "2.1.8.2:5060,1000,0.000,1,UDP\n"
           "2.1.8.3:5060,100,0.009,1,TCP\n"
           "" },

         // Test to see that our own name _does_not override priorities
         { "test31", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.8.2:5060,1000,0.000,1,UDP\n"
           "2.1.8.3:5060,100,0.009,1,TCP\n"
           "2.1.8.1:5060,1,0.244,2,TCP\n"
           "2.1.8.1:5060,1,0.225,2,UDP\n"
           "" },

         // Test to see that name preference works among low-priority alternatives
         { "test32", "sip", OsSocket::UNKNOWN, -1, NULL,
           "2.1.8.2:5060,1,0.174,1,UDP\n"
           "2.1.8.1:5060,1,0.244,2,TCP\n"
           "2.1.8.1:5060,1,0.225,2,UDP\n"
           "2.1.8.3:5060,100,0.009,2,TCP\n"
           "" },

         // Numeric IPv6 address.
         // Yields no destinations, because we haven't implemented IPv6 lookup,
         // but this ensures that the URI can be processed without faulting.
         { "[a0:32:44::99]", "sip", OsSocket::UNKNOWN, -1, NULL,
           "" },

      };

      // Flag for whether any test cases have failed.
      int failure_seen = 0;

      // Set our own host name to testself.example.com
      SipSrvLookup::setOwnHostname("testself.example.com");
      
      // Set up the option values we desire for all tests.
      SipSrvLookup::setOption(SipSrvLookup::OptionCodeSortAnswers, 1);
      SipSrvLookup::setOption(SipSrvLookup::OptionCodeSortServers, 1);
#if 0
      SipSrvLookup::setOption(SipSrvLookup::OptionCodePrintAnswers, 1);
#endif
      // Save all option values.
      int saved_options[SipSrvLookup::OptionCodeLast+1];
      for (int i = SipSrvLookup::OptionCodeFirst;
           i <= SipSrvLookup::OptionCodeLast; i++)
      {
         saved_options[i] =
            SipSrvLookup::getOption((SipSrvLookup::OptionCode) i);
      }

      // Loop through all the test cases.
      for (unsigned int test_no = 0;
           test_no < sizeof (tests) / sizeof (tests[0]); test_no++)
      {
         // Reset the random number generator before every test to ensure
         // the results are reproducible even if they are rearranged.
         srand(1);              // 1 is the default seed.

         // Restore the saved option values.
         for (int j = SipSrvLookup::OptionCodeFirst;
              j <= SipSrvLookup::OptionCodeLast; j++)
         {
            SipSrvLookup::setOption((SipSrvLookup::OptionCode) j,
                                    saved_options[j]);
         }
         // Set the specified options.
         int *option_list = tests[test_no].options;
         if (option_list != NULL)
         {
            for (; option_list[0] != SipSrvLookup::OptionCodeNone;
                 option_list += 2)
            {
               SipSrvLookup::setOption((SipSrvLookup::OptionCode)
                                       option_list[0],
                                       option_list[1]);
            }
         }

         // Call SipSrvLookup::servers.
         server_t* p;
         p = SipSrvLookup::servers(tests[test_no].name, tests[test_no].service,
                                   tests[test_no].type, tests[test_no].port);

         // Construct a monster string containing the crucial parts of the
         // results.
         result_string[0] = '\0';
         for (server_t* q = p; q->isValidServerT(); q++)
         {
            // Append "IP:port,weight,score,priority,proto\n" to
            // result_string.
            UtlString ip_addr;
            q->getIpAddressFromServerT(ip_addr);
            sprintf(result_string + strlen(result_string),
                    "%s:%d,%u,%.3f,%u,%s\n",
                    ip_addr.data(),
                    q->getPortFromServerT(),
                    q->getWeightFromServerT(),
                    q->getScoreFromServerT(),
                    q->getPriorityFromServerT(),
                    printable_proto(q->getProtocolFromServerT()));
         }
         // Make sure we haven't overflowed result_string.
         CPPUNIT_ASSERT(strlen(result_string) < sizeof (result_string));

         // Check whether we saw unexpected results.
         int unexpected = (strcmp(result_string, tests[test_no].expected) != 0);
#ifdef DEBUG
         // For debugging, force printing of results.
         unexpected = TRUE;
#endif /* DEBUG */

         // If the actual results did not match the expected results,
         // print both.
         if (unexpected) {
            failure_seen = 1;

            printf("\nSipSrvLookup::servers(\"%s\", \"%s\", OsSocket::%s, "
                   "%d) returns:\n",
                   tests[test_no].name, tests[test_no].service,
                   printable_proto(tests[test_no].type), tests[test_no].port);
            for (server_t* q = p; q->isValidServerT(); q++)
            {
               UtlString host;
               q->getHostNameFromServerT(host);
               UtlString ip_addr;
               q->getIpAddressFromServerT(ip_addr);
               printf("\tSipSrvLookupTest::lookup host = '%s', IP addr = '%s', "
                      "port = %d, weight = %u, score = %f, "
                      "priority = %u, proto = %s\n",
                      host.data(), ip_addr.data(),
                      q->getPortFromServerT(),
                      q->getWeightFromServerT(),
                      q->getScoreFromServerT(),
                      q->getPriorityFromServerT(),
                      printable_proto(q->getProtocolFromServerT()));
            }
            printf("\t[END]\n");
            printf("\nFailed test seq. no. %d, named '%s'\n", test_no,
                   tests[test_no].name);
            printf("Expected results were:\n%s\n", tests[test_no].expected);
            printf("Actual results were:\n%s\n", result_string);
         }
         else
         {
#           ifdef TEST_PRINT
            printf("Passed %d: %s\n", test_no, tests[test_no].name);
#           endif
         }

         // Free the results list.
         delete[] p;
      }

      // Kill the test named.
      // (Do this before checking failure_seen, as CPPUNIT_ASSERT will
      // terminate the function if it fails.)
      int kill_return = kill(named_pid, SIGTERM);
#     ifdef TEST_PRINT
      printf("kill(%ld, SIGTERM) = %d\n", (long)named_pid, kill_return);
#     endif
      CPPUNIT_ASSERT(kill_return == 0);

      // Delete the named configuration file.
      unlink(config_file);

      // Report error if any test has failed.
      CPPUNIT_ASSERT(!failure_seen);
#else /* NAMED_PROGRAM */
      printf("... not executed because 'configure' could not find 'named' or 'named' was not executable.\n");
#endif /* NAMED_PROGRAM */
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipSrvLookupTest);

// Get a printable representation of a protocol value.
const char* printable_proto(OsSocket::IpProtocolSocketType type)
{
   const char* s;
   switch (type)
   {
   case OsSocket::UNKNOWN:
      s = "UNKNOWN";
      break;
   case OsSocket::TCP:
      s = "TCP";
      break;
   case OsSocket::UDP:
      s = "UDP";
      break;
   case OsSocket::SSL_SOCKET:
      s = "TLS";
      break;
   default:
      s = "***bad value***";
      break;
   }
   return s;
}
