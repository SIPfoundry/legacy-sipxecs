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
#include <os/OsSysLog.h>
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
          const char* hexoutput;
          const char* b64output;
       };
       struct test tests[] = {
          // Varying length strings of 0's.
          // 0
          { "",
            "d41d8cd98f00b204e9800998ecf8427e",
            "1B2M2Y8AsgTpgAmY7PhCfg"
          },
          // 1
          { "0",
            "cfcd208495d565ef66e7dff9f98764da",
            "z80ghJXVZe9m59`5_Ydk2g"
          },
          // 2
          { "00",
            "b4b147bc522828731f1a016bfa72c073",
            "tLFHvFIoKHMfGgFr_nLAcw"
          },
          // 3
          { "0000",
            "4a7d1ed414474e4033ac29ccb8653d9b",
            "Sn0e1BRHTkAzrCnMuGU9mw"
          },
          // 4
          { "00000000",
            "dd4b21e9ef71e1291183a46b913ae6f2",
            "3Ush6e9x4SkRg6RrkTrm8g"
          },
          // 5
          { "0000000000000000",
            "1e4a1b03d1b6cd8a174a826f76e009f4",
            "HkobA9G2zYoXSoJvduAJ9A"
          },
          // 6
          { "00000000000000000000000000000000",
            "cd9e459ea708a948d5c2f5a6ca8838cf",
            "zZ5FnqcIqUjVwvWmyog4zw"
          },
          // 7
          { "0000000000000000000000000000000000000000000000000000000000000000",
            "10eab6008d5642cf42abd2aa41f847cb",
            "EOq2AI1WQs9Cq9KqQfhHyw"
          },
          // 8
          { "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
            "aa70aaf67b3bab5029b76cee92e18afe",
            "qnCq9ns7q1Apt2zukuGK`g"
          },
          // Varying length strings of the ASCII characters.
          // 9
          { " ",
            "7215ee9c7d9dc229d2921a40e899ec5f",
            "chXunH2dwinSkhpA6JnsXw"
          },
          // 10
          { " !",
            "220c1c252883eadad5590fc9e6a61739",
            "IgwcJSiD6trVWQ`J5qYXOQ"
          },
          // 11
          { " !\"#",
            "1a8bed44f8555d8318157f1e649a99b5",
            "GovtRPhVXYMYFX8eZJqZtQ"
          },
          // 12
          { " !\"#$%&'",
            "04281adcae556d29c613104883b36133",
            "BCga3K5VbSnGExBIg7NhMw"
          },
          // 13
          { " !\"#$%&'()*+,-./",
            "35ba6d08f0e34c15b9d0b09998960eb6",
            "NbptCPDjTBW50LCZmJYOtg"
          },
          // 14
          { " !\"#$%&'()*+,-./0123456789:;<=>?",
            "bf61e899560fabde2f6d76f405a6eb70",
            "v2HomVYPq94vbXb0BabrcA"
          },
          // 15
          { " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_",
            "c654a1965b312ddee3bd884e044a7658",
            "xlShllsxLd7jvYhOBEp2WA"
          },
          // 16
          { " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ",
            "6c16dd02a16a44568a8fd4b9bf2fdddd",
            "bBbdAqFqRFaKj9S5vy`d3Q"
          },
          // 17
          // Strings of length 65:  Since MD5 processes chars in blocks
          // of 64, this tests behavior across block boundaries.
          { "00000000000000000000000000000000000000000000000000000000000000000",
            "f8c702aaa8c658413a4efb3a614d7707",
            "_McCqqjGWEE6Tvs6YU13Bw"
          },
          // 18
          { " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`",
            "bf35d3d5ee617924bbaabfef05b78d30",
            "vzXT1e5heSS7qr`vBbeNMA"
          },
          // 19
          // A couple of more normal strings.
          { "john.salesman:sales/example.com:5+5=10",
            "91ffcb1f8353fe113ccf7169ee27402e",
            "kf`LH4NT`hE8z3Fp7idALg"
          },
          // 20
          { "GET:/private/prices.html",
            "254bd53db6966fa1387fa1973bb5e53c",
            "JUvVPbaWb6E4f6GXO7XlPA"
          }
       };

       // For the static encode method, use the same output string for all calls,
       // to test that when NetMd5Codec writes its output into the string,
       // it removes any previous contents (rather than appending to
       // them).
       UtlString encodedString;
       bool allPassed=true;
       for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]); i++)
       {
          NetMd5Codec::encode(tests[i].input, encodedString);
          if (encodedString.compareTo(tests[i].hexoutput) != 0)
          {
             allPassed=false;
             OsSysLog::add(FAC_SIP, PRI_ERR, "md5 encode hex test %d, string: %s\n  expected: %s\n  returned: %s",
                           i, tests[i].input, tests[i].hexoutput, encodedString.data());
          }

          NetMd5Codec::encodeBase64Sig(tests[i].input, encodedString);
          if (encodedString.compareTo(tests[i].b64output) != 0)
          {
             OsSysLog::add(FAC_SIP, PRI_ERR, "md5 encode b64 test %d, string: %s\n  expected: %s\n  returned: %s",
                           i, tests[i].input, tests[i].b64output, encodedString.data());
             allPassed=false;
          }
       }
       CPPUNIT_ASSERT(allPassed);
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
            // 0
            { 2, { "", "" },
              "d41d8cd98f00b204e9800998ecf8427e" },
            // 1
            { 2, { "0", "" },
              "cfcd208495d565ef66e7dff9f98764da" },
            // 2
            { 2, { "0", "0" },
              "b4b147bc522828731f1a016bfa72c073" },
            // 3
            { 3, { "00", "0", "0" },
              "4a7d1ed414474e4033ac29ccb8653d9b" },
            // 4
            { 2, { "000", "00000" },
              "dd4b21e9ef71e1291183a46b913ae6f2" },
            // 5
            { 1, { "0000000000000000" },
              "1e4a1b03d1b6cd8a174a826f76e009f4" },
            // 6
            { 3, { "", "0000000", "0000000000000000000000000" },
              "cd9e459ea708a948d5c2f5a6ca8838cf" },
            // 7
            { 2, { "000000000000000", "0000000000000000000000000000000000000000000000000" },
              "10eab6008d5642cf42abd2aa41f847cb" },
            // 8
            { 1, { "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" },
              "aa70aaf67b3bab5029b76cee92e18afe" },
            // Varying length strings of the ASCII characters.
            // 8
            { 2, { " ", "" },
              "7215ee9c7d9dc229d2921a40e899ec5f" },
            // 9
            { 2, { " ", "!" },
              "220c1c252883eadad5590fc9e6a61739" },
            // 10
            { 2, { " !\"", "#" },
              "1a8bed44f8555d8318157f1e649a99b5" },
            // 11
            { 2, { " !\"#", "$%&'" },
              "04281adcae556d29c613104883b36133" },
            // 12
            { 2, { " ", "!\"#$%&'()*+,-./" },
              "35ba6d08f0e34c15b9d0b09998960eb6" },
            // 13
            { 2, { " !\"#$%&'()*+,-.", "/0123456789:;<=>?" },
              "bf61e899560fabde2f6d76f405a6eb70" },
            // 14
            { 2, { " !\"#$%&'()*+,-./0123456789:;<=>?@A", "BCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_" },
              "c654a1965b312ddee3bd884e044a7658" },
            // 15
            { 2, { " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDE", "FGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ " },
              "6c16dd02a16a44568a8fd4b9bf2fdddd" },
            // Strings of length 65:  Since MD5 processes chars in blocks
            // of 64, this tests behavior across block boundaries.
            // 16
            { 2, { "00000000000000000000000000000000000000000000000000000000000000000", "00000000000000000000000000000000000000000000000000000000000000000" },
              "c7f6509e1fde3ebc0c246b98ca17ca53" },
            // 17
            { 1, { " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`" },
              "bf35d3d5ee617924bbaabfef05b78d30" },
            // A couple of more normal strings.
            // 18
            { 5, { "john.salesman", ":", "sales/example.com", ":", "5+5=10" },
              "91ffcb1f8353fe113ccf7169ee27402e" },
            // 19
            { 1, { "GET:/private/prices.html" },
              "254bd53db6966fa1387fa1973bb5e53c" }
         };

         UtlString prefix("prefix/");
         bool allPassed=true;
         for (unsigned int testcase = 0;
              testcase < sizeof(tests) / sizeof(struct test);
              testcase++
              )
         {
            NetMd5Codec md5;
            NetMd5Codec md5sig;
            UtlString concatenatedInput;

            for (int input = 0; input < tests[testcase].numInputs; input++)
            {
               concatenatedInput.append(tests[testcase].inputs[input]);

               md5.hash(tests[testcase].inputs[input],
                        strlen(tests[testcase].inputs[input])
                        );
               md5sig.hash(tests[testcase].inputs[input],
                           strlen(tests[testcase].inputs[input])
                           );
            }
            UtlString actualOutputSegmented(prefix);
            md5.appendHashValue(actualOutputSegmented);

            UtlString prefixedOutputConcatenated(prefix);
            UtlString actualOutputConcatenated;
            NetMd5Codec::encode(concatenatedInput.data(), actualOutputConcatenated);
            prefixedOutputConcatenated.append(actualOutputConcatenated);

            if (actualOutputSegmented.compareTo(prefixedOutputConcatenated) != 0)
            {
               allPassed=false;
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "md5 multiple input test %d, segmented: %s\n  concatenated: %s",
                             testcase, actualOutputSegmented.data(), prefixedOutputConcatenated.data());
            }

            UtlString sigOutputSegmented(prefix);
            md5sig.appendBase64Sig(sigOutputSegmented);

            UtlString sigPrefixedOutputConcatenated(prefix);
            UtlString sigOutputConcatenated;
            NetMd5Codec::encodeBase64Sig(concatenatedInput.data(), sigOutputConcatenated);
            sigPrefixedOutputConcatenated.append(sigOutputConcatenated);

            if (sigOutputSegmented.compareTo(sigPrefixedOutputConcatenated) != 0)
            {
               allPassed=false;
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "md5 multiple input test %d, segmented: %s\n  concatenated: %s",
                             testcase, sigOutputSegmented.data(), sigPrefixedOutputConcatenated.data());
            }

            UtlString expectedOutput(prefix);
            expectedOutput.append(tests[testcase].output);

            if (actualOutputSegmented.compareTo(expectedOutput) != 0)
            {
               allPassed=false;
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "md5 multiple input test %d, expected: %s\n  received: %s",
                             testcase, actualOutputSegmented.data(), expectedOutput.data());
            }
         }

         CPPUNIT_ASSERT(allPassed);


      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetMd5CodecTest);
