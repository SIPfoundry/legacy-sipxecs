//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/NetMd5Codec.h>

/**
 * Unittest for NetMd5Codec
 */
class NetMd5CodecTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(NetMd5CodecTest);
    CPPUNIT_TEST(testSingleInputEncode);
    CPPUNIT_TEST(testMultipleInput);
    CPPUNIT_TEST_SUITE_END();


public:
    void testSingleInputEncode()
    {
       // Table of test cases.
       struct test {
          const char* input;
          const char* output;
       };
       struct test tests[] = {
          // Varying length strings of 0's.
          { "",
            "d41d8cd98f00b204e9800998ecf8427e" },
          { "0",
            "cfcd208495d565ef66e7dff9f98764da" },
          { "00",
            "b4b147bc522828731f1a016bfa72c073" },
          { "0000",
            "4a7d1ed414474e4033ac29ccb8653d9b" },
          { "00000000",
            "dd4b21e9ef71e1291183a46b913ae6f2" },
          { "0000000000000000",
            "1e4a1b03d1b6cd8a174a826f76e009f4" },
          { "00000000000000000000000000000000",
            "cd9e459ea708a948d5c2f5a6ca8838cf" },
          { "0000000000000000000000000000000000000000000000000000000000000000",
            "10eab6008d5642cf42abd2aa41f847cb" },
          { "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
            "aa70aaf67b3bab5029b76cee92e18afe" },
          // Varying length strings of the ASCII characters.
          { " ",
            "7215ee9c7d9dc229d2921a40e899ec5f" },
          { " !",
            "220c1c252883eadad5590fc9e6a61739" },
          { " !\"#",
            "1a8bed44f8555d8318157f1e649a99b5" },
          { " !\"#$%&'",
            "04281adcae556d29c613104883b36133" },
          { " !\"#$%&'()*+,-./",
            "35ba6d08f0e34c15b9d0b09998960eb6" },
          { " !\"#$%&'()*+,-./0123456789:;<=>?",
            "bf61e899560fabde2f6d76f405a6eb70" },
          { " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_",
            "c654a1965b312ddee3bd884e044a7658" },
          { " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ",
            "6c16dd02a16a44568a8fd4b9bf2fdddd" },
          // Strings of length 65:  Since MD5 processes chars in blocks
          // of 64, this tests behavior across block boundaries.
          { "00000000000000000000000000000000000000000000000000000000000000000",
            "f8c702aaa8c658413a4efb3a614d7707" },
          { " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`",
            "bf35d3d5ee617924bbaabfef05b78d30" },
          // A couple of more normal strings.
          { "john.salesman:sales/example.com:5+5=10",
            "91ffcb1f8353fe113ccf7169ee27402e" },
          { "GET:/private/prices.html",
            "254bd53db6966fa1387fa1973bb5e53c" }
       };

       // For the static encode method, use the same output string for all calls, 
       // to test that when NetMd5Codec writes its output into the string,
       // it removes any previous contents (rather than appending to
       // them).
       UtlString encodedString;
       char msg[1000];

       for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]); i++)
       {
          sprintf(msg, "md5 encode test %d, string: %s\n",
                  i, tests[i].input);
          NetMd5Codec::encode(tests[i].input, encodedString);
          CPPUNIT_ASSERT_MESSAGE(msg,
                                 encodedString.compareTo(tests[i].output) == 0);
       }
    }

   void testMultipleInput()
      {
         // Table of test cases.
         struct test {
            int          numInputs;
            const char*  inputs[6];
            const char*  output;
         };
         struct test tests[] = {
            // Varying length strings of 0's.
            { 2, { "", "" },
              "d41d8cd98f00b204e9800998ecf8427e" },
            { 2, { "0", "" },
              "cfcd208495d565ef66e7dff9f98764da" },
            { 2, { "0", "0" },
              "b4b147bc522828731f1a016bfa72c073" },
            { 3, { "00", "0", "0" },
              "4a7d1ed414474e4033ac29ccb8653d9b" },
            { 2, { "000", "00000" },
              "dd4b21e9ef71e1291183a46b913ae6f2" },
            { 1, { "0000000000000000" },
              "1e4a1b03d1b6cd8a174a826f76e009f4" },
            { 3, { "", "0000000", "0000000000000000000000000" },
              "cd9e459ea708a948d5c2f5a6ca8838cf" },
            { 2, { "000000000000000", "0000000000000000000000000000000000000000000000000" },
              "10eab6008d5642cf42abd2aa41f847cb" },
            { 1, { "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" },
              "aa70aaf67b3bab5029b76cee92e18afe" },
            // Varying length strings of the ASCII characters.
            { 2, { " ", "" },
              "7215ee9c7d9dc229d2921a40e899ec5f" },
            { 2, { " ", "!" },
              "220c1c252883eadad5590fc9e6a61739" },
            { 2, { " !\"", "#" },
              "1a8bed44f8555d8318157f1e649a99b5" },
            { 2, { " !\"#", "$%&'" },
              "04281adcae556d29c613104883b36133" },
            { 2, { " ", "!\"#$%&'()*+,-./" },
              "35ba6d08f0e34c15b9d0b09998960eb6" },
            { 2, { " !\"#$%&'()*+,-.", "/0123456789:;<=>?" },
              "bf61e899560fabde2f6d76f405a6eb70" },
            { 2, { " !\"#$%&'()*+,-./0123456789:;<=>?@A", "BCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_" },
              "c654a1965b312ddee3bd884e044a7658" },
            { 2, { " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDE", "FGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ " },
              "6c16dd02a16a44568a8fd4b9bf2fdddd" },
            // Strings of length 65:  Since MD5 processes chars in blocks
            // of 64, this tests behavior across block boundaries.
            { 2, { "00000000000000000000000000000000000000000000000000000000000000000", "00000000000000000000000000000000000000000000000000000000000000000" },
              "c7f6509e1fde3ebc0c246b98ca17ca53" },
            { 1, { " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`" },
              "bf35d3d5ee617924bbaabfef05b78d30" },
            // A couple of more normal strings.
            { 5, { "john.salesman", ":", "sales/example.com", ":", "5+5=10" },
              "91ffcb1f8353fe113ccf7169ee27402e" },
            { 1, { "GET:/private/prices.html" },
              "254bd53db6966fa1387fa1973bb5e53c" }
         };

         UtlString prefix("prefix/");
         for (unsigned int testcase = 0;
              testcase < sizeof(tests) / sizeof(struct test);
              testcase++
              )
         {
            NetMd5Codec md5;

            for (int input = 0; input < tests[testcase].numInputs; input++)
            {
               md5.hash(tests[testcase].inputs[input],
                        strlen(tests[testcase].inputs[input])
                        );
            }
            UtlString actualOutput(prefix);
            md5.appendHashValue(actualOutput);

            UtlString expectedOutput(prefix);
            expectedOutput.append(tests[testcase].output);
            
            char msg[1024];
            sprintf(msg, "test %d did not match",
                    testcase
                    );
            CPPUNIT_ASSERT_MESSAGE(msg, expectedOutput.compareTo(actualOutput) == 0);
         }
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetMd5CodecTest);
