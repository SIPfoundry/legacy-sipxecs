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

#include <os/OsTime.h>

/**
 * Unittest for OsTime
 */
class OsTimeTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsTimeTest);
    CPPUNIT_TEST(testSeconds);
    CPPUNIT_TEST(testCopyConstructor);
    CPPUNIT_TEST(testOverflow);
    CPPUNIT_TEST(testCompare);
    CPPUNIT_TEST_SUITE_END();


public:
    void testSeconds()
    {
        const char *msg;
        OsTime *pInterval1;

        msg = "test the default constructor (if implemented)";
        pInterval1 = new OsTime();
        CPPUNIT_ASSERT_MESSAGE(msg, 0 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 0 == pInterval1->usecs());
        delete pInterval1;

        // test other constructors (if implemented)
        msg = "if a constructor parameter is used to set information in \
an ancestor class, then verify it gets set correctly (i.e., via ancestor \
class accessor method.";
        msg = "try giving the constructor a positive \"millisecond\" value";
        pInterval1 = new OsTime(3010);
        CPPUNIT_ASSERT_MESSAGE(msg, 3     == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 10000 == pInterval1->usecs());
        delete pInterval1;

        msg = "try giving the constructor a negative \"millisecond\" value";
        pInterval1 = new OsTime(-3010);
        CPPUNIT_ASSERT_MESSAGE(msg, -3010  == pInterval1->cvtToMsecs());
        delete pInterval1;

        msg = "try giving the constructor a positive \"second\" and \"microsecond\" values";
        pInterval1 = new OsTime(30, 10);
        CPPUNIT_ASSERT_MESSAGE(msg, 30 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 10 == pInterval1->usecs());
        delete pInterval1;

        msg = "try giving the constructor a negative \"seconds\" value";
        pInterval1 = new OsTime(-20, 10);
        CPPUNIT_ASSERT_MESSAGE(msg, -20 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 10 == pInterval1->usecs());
        delete pInterval1;

        msg = "try giving the constructor a negative \"microseconds\" value";
        pInterval1 = new OsTime(20, -30);
        CPPUNIT_ASSERT_MESSAGE(msg, 19 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, OsTime::USECS_PER_SEC - 30 == pInterval1->usecs());
        delete pInterval1;

        msg = "try giving the constructor a \"microseconds\" value greater \
than one sec";
        pInterval1 = new OsTime(20, 2000001);
        CPPUNIT_ASSERT_MESSAGE(msg, 22 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 1 == pInterval1->usecs());
        delete pInterval1;

        msg = "try giving the constructor a \"microseconds\" value less \
than one sec";
        pInterval1 = new OsTime(20, -2000001);
        CPPUNIT_ASSERT_MESSAGE(msg, 17 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, OsTime::USECS_PER_SEC - 1 == pInterval1->usecs());
        delete pInterval1;

        msg = "try giving the constructor a \"microseconds\" value less \
than one sec";
        pInterval1 = new OsTime(20, -1999999);
        CPPUNIT_ASSERT_MESSAGE(msg, 18 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 1 == pInterval1->usecs());
        delete pInterval1;
    }

    void testCopyConstructor()
    {
        const char *msg;
        OsTime *pInterval1;
        OsTime *pInterval2;

        msg = "test the copy constructor applied to an object created \
using the default constructor";
        pInterval1 = new OsTime();
        pInterval2 = new OsTime(*pInterval1);
        CPPUNIT_ASSERT_MESSAGE(msg, 0 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 0 == pInterval1->usecs());

        CPPUNIT_ASSERT_MESSAGE(msg, pInterval2->seconds() == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, pInterval2->usecs() == pInterval1->usecs());
        delete pInterval1;
        delete pInterval2;

        msg = "test the copy constructor applied to an object create using a \
non-default constructor";
        pInterval1 = new OsTime(30, 10);
        pInterval2 = new OsTime(*pInterval1);
        CPPUNIT_ASSERT_MESSAGE(msg, 30 == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, 10 == pInterval1->usecs());
        CPPUNIT_ASSERT_MESSAGE(msg, pInterval2->seconds() == pInterval1->seconds());
        CPPUNIT_ASSERT_MESSAGE(msg, pInterval2->usecs() == pInterval1->usecs());
        delete pInterval1;
        delete pInterval2;
    }

    void testOverflow()
    {
       OsTime t900(0, 900000);

       OsTime t1(1, 0);
       OsTime t2 = t1 + t900;
       CPPUNIT_ASSERT_MESSAGE("t2 = (1, 900000)",
                              t2.seconds() == 1 && t2.usecs() == 900000);

       OsTime t3 = t2 + t900;
       CPPUNIT_ASSERT_MESSAGE("t3 = (2, 800000)",
                              t3.seconds() == 2 && t3.usecs() == 800000);

       OsTime t4 = t2 + t3;
       CPPUNIT_ASSERT_MESSAGE("t4 = (4, 700000)",
                              t4.seconds() == 4 && t4.usecs() == 700000);

       OsTime t10(-1, 0);
       OsTime t11 = t10 + t900;
       CPPUNIT_ASSERT_MESSAGE("t11 = (-1, 900000)",
                              t11.seconds() == -1 && t11.usecs() == 900000);

       OsTime t12 = t11 + t900;
       CPPUNIT_ASSERT_MESSAGE("t12 = (0, 800000)",
                              t12.seconds() == 0 && t12.usecs() == 800000);

       OsTime t21(1, 0);
       OsTime t22 = t21 - t900;
       CPPUNIT_ASSERT_MESSAGE("t22 = (0, 100000)",
                              t22.seconds() == 0 && t22.usecs() == 100000);

       OsTime t23 = t22 - t900;
       CPPUNIT_ASSERT_MESSAGE("t23 = (-1, 200000)",
                              t23.seconds() == -1 && t23.usecs() == 200000);

       OsTime t24 = t22 - t23;
       CPPUNIT_ASSERT_MESSAGE("t24 = (0, 900000)",
                              t24.seconds() == 0 && t24.usecs() == 900000);

       OsTime t30(-1, 0);
       OsTime t31 = t30 - t900;
       CPPUNIT_ASSERT_MESSAGE("t31 = (-2, 100000)",
                              t31.seconds() == -2 && t31.usecs() == 100000);

       OsTime t100(0, 100000);
       OsTime t32 = t31 - t100;
       CPPUNIT_ASSERT_MESSAGE("t32 = (-2, 0)",
                              t32.seconds() == -2 && t32.usecs() == 0);
    }

    void testCompare()
    {
       // Array of integer pairs to initialize an array of OsTime's.
       // Must be in increasing order.
       int array[][2] = {
          {-10, 0},
          {-10, 1},
          {-10, 999999},
          {-2, 0},
          {-2, 1},
          {-2, 500000},
          {-1, 0},
          {-1, 1},
          {-1, 999999},
          {0, 0},
          {0, 1},
          {0, 999999},
          {1, 0},
          {1, 1}
       };
       #define N (sizeof (array) / sizeof (array[0]))
       OsTime times[N];
       unsigned int i, j;
       char msg[1000];

       // Initialize the array of OsTime's.
       for (i = 0; i < N; i++)
       {
          times[i] = OsTime(array[i][0], array[i][1]);
       }

       // Test the comparison operators.
       for (i = 0; i < N; i++)
       {
          for (j = 0; j < N; j++)
          {
             sprintf(msg, "Comparing times[%d] and times[%d]", i, j);
             #if 0
             fprintf(stderr, "* %d %d.%06d %d %d.%06d\n",
                     i, times[i].seconds(), times[i].usecs(),
                     j, times[j].seconds(), times[j].usecs());
             #endif // 0
             CPPUNIT_ASSERT_MESSAGE(msg,
                                    (i == j) == (times[i] == times[j]));
             CPPUNIT_ASSERT_MESSAGE(msg,
                                    (i != j) == (times[i] != times[j]));
             CPPUNIT_ASSERT_MESSAGE(msg,
                                    (i < j) == (times[i] < times[j]));
             CPPUNIT_ASSERT_MESSAGE(msg,
                                    (i <= j) == (times[i] <= times[j]));
             CPPUNIT_ASSERT_MESSAGE(msg,
                                    (i > j) == (times[i] > times[j]));
             CPPUNIT_ASSERT_MESSAGE(msg,
                                    (i >= j) == (times[i] >= times[j]));
          }
       }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsTimeTest);
