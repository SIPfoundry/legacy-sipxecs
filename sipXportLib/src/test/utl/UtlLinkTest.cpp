//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include "utl/UtlLink.h"
#include "utl/UtlInt.h"
#include <sipxunit/TestUtilities.h>

/// Unit test of the UtlLink and UtlLinkPool classes.

static UtlInt data1(1);
static UtlInt data2(2);
static UtlInt data3(3);


/// Unit test of the UtlLink, UtlLinkPool, and UtlChain classes.
class UtlLinkTest :
   public CppUnit::TestCase,
   public UtlLink
{
   CPPUNIT_TEST_SUITE(UtlLinkTest);
   /* CPPUNIT_TEST(testEmptyPool); - removed because with the addition of test logging
                                     the pool has already been used by parts of the OsSysLog
                                     infrastructure, so it's not empty.
                                     If you really really want to run this test with this
                                     enabled, see sipXportLib/src/test/sipxunit/TestRunner.cpp
                                     and comment out the call to install the TestOsSysLogListener.
   */
   CPPUNIT_TEST(testLinkBefore);
   CPPUNIT_TEST(testListBefore);
   CPPUNIT_TEST(testLinkAfter);
   CPPUNIT_TEST(testListAfter);
   CPPUNIT_TEST(testLinkReuse);
   CPPUNIT_TEST_SUITE_END();

private:

public:

   void testEmptyPool()
      {
         CPPUNIT_ASSERT(totalAllocated() == 0);
      }

   void testLinkBefore()
      {
         UtlChain start;

         UtlLink* link3 = UtlLink::before(&start, &data3);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(link3->prev() == NULL);
         CPPUNIT_ASSERT(link3->next() == &start);
         CPPUNIT_ASSERT(start.prev == link3);
         CPPUNIT_ASSERT(start.next == NULL);

         UtlLink* link2 = UtlLink::before(link3, &data2);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);
         CPPUNIT_ASSERT(link2->data   == &data2 && data2.getValue() == 2);

         CPPUNIT_ASSERT(link2->prev() == NULL);
         CPPUNIT_ASSERT(link2->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link2);
         CPPUNIT_ASSERT(link3->next() == &start);
         CPPUNIT_ASSERT(start.prev == link3);
         CPPUNIT_ASSERT(start.next == NULL);

         UtlLink* link1 = UtlLink::before(link2, &data1);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);
         CPPUNIT_ASSERT(link2->data   == &data2 && data2.getValue() == 2);
         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(link1->prev() == NULL);
         CPPUNIT_ASSERT(link1->next() == link2);
         CPPUNIT_ASSERT(link2->prev() == link1);
         CPPUNIT_ASSERT(link2->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link2);
         CPPUNIT_ASSERT(link3->next() == &start);
         CPPUNIT_ASSERT(start.prev == link3);
         CPPUNIT_ASSERT(start.next == NULL);

         void* returnedData;

         returnedData = link2->unlink();
         CPPUNIT_ASSERT(returnedData == &data2 && data2.getValue() == 2);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);
         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(link1->prev() == NULL);
         CPPUNIT_ASSERT(link1->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link1);
         CPPUNIT_ASSERT(link3->next() == &start);
         CPPUNIT_ASSERT(start.prev == link3);
         CPPUNIT_ASSERT(start.next == NULL);

         returnedData = link1->unlink();
         CPPUNIT_ASSERT(returnedData == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(link3->prev() == NULL);
         CPPUNIT_ASSERT(link3->next() == &start);
         CPPUNIT_ASSERT(start.prev == link3);
         CPPUNIT_ASSERT(start.next == NULL);

         returnedData = link3->unlink();
         CPPUNIT_ASSERT(returnedData == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(start.prev == NULL);
         CPPUNIT_ASSERT(start.next == NULL);
      }

   void testListBefore()
      {
         UtlChain list;

         UtlLink* link1 = UtlLink::listBefore(&list, NULL, &data1);

         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(link1->prev() == NULL);
         CPPUNIT_ASSERT(link1->next() == NULL);
         CPPUNIT_ASSERT(list.prev == link1);
         CPPUNIT_ASSERT(list.next == link1);

         UtlLink* link3 = UtlLink::listBefore(&list, NULL, &data3);

         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);
         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(list.next == link1);
         CPPUNIT_ASSERT(link1->prev() == NULL);
         CPPUNIT_ASSERT(link1->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link1);
         CPPUNIT_ASSERT(link3->next() == NULL);
         CPPUNIT_ASSERT(list.prev == link3);

         UtlLink* link2 = UtlLink::listBefore(&list, link3, &data2);

         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);
         CPPUNIT_ASSERT(link2->data   == &data2 && data2.getValue() == 2);
         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(list.next == link1);
         CPPUNIT_ASSERT(link1->prev() == NULL);
         CPPUNIT_ASSERT(link1->next() == link2);
         CPPUNIT_ASSERT(link2->prev() == link1);
         CPPUNIT_ASSERT(link2->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link2);
         CPPUNIT_ASSERT(link3->next() == NULL);
         CPPUNIT_ASSERT(list.prev == link3);

         void* returnedData;

         returnedData = link2->detachFrom(&list);
         CPPUNIT_ASSERT(returnedData == &data2 && data2.getValue() == 2);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);
         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(list.next == link1);
         CPPUNIT_ASSERT(link1->prev() == NULL);
         CPPUNIT_ASSERT(link1->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link1);
         CPPUNIT_ASSERT(link3->next() == NULL);
         CPPUNIT_ASSERT(list.prev == link3);

         returnedData = link1->detachFrom(&list);
         CPPUNIT_ASSERT(returnedData == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(list.next == link3);
         CPPUNIT_ASSERT(link3->prev() == NULL);
         CPPUNIT_ASSERT(link3->next() == NULL);
         CPPUNIT_ASSERT(list.prev == link3);

         returnedData = link3->detachFrom(&list);
         CPPUNIT_ASSERT(returnedData == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(list.prev == NULL);
         CPPUNIT_ASSERT(list.next == NULL);
      }


   void testLinkAfter()
      {
         UtlChain start;

         UtlLink* link1 = UtlLink::after(&start, &data1);

         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(start.prev == NULL);
         CPPUNIT_ASSERT(start.next == link1);
         CPPUNIT_ASSERT(link1->prev() == &start);
         CPPUNIT_ASSERT(link1->next() == NULL);

         UtlLink* link2 = UtlLink::after(link1, &data2);

         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);
         CPPUNIT_ASSERT(link2->data   == &data2 && data2.getValue() == 2);

         CPPUNIT_ASSERT(start.prev == NULL);
         CPPUNIT_ASSERT(start.next == link1);
         CPPUNIT_ASSERT(link1->prev() == &start);
         CPPUNIT_ASSERT(link1->next() == link2);
         CPPUNIT_ASSERT(link2->prev() == link1);
         CPPUNIT_ASSERT(link2->next() == NULL);

         UtlLink* link3 = UtlLink::after(link2, &data3);

         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);
         CPPUNIT_ASSERT(link2->data   == &data2 && data2.getValue() == 2);
         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(start.prev == NULL);
         CPPUNIT_ASSERT(start.next == link1);
         CPPUNIT_ASSERT(link1->prev() == &start);
         CPPUNIT_ASSERT(link1->next() == link2);
         CPPUNIT_ASSERT(link2->prev() == link1);
         CPPUNIT_ASSERT(link2->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link2);
         CPPUNIT_ASSERT(link3->next() == NULL);

         void* returnedData;

         returnedData = link2->unlink();
         CPPUNIT_ASSERT(returnedData == &data2 && data2.getValue() == 2);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);
         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(start.prev == NULL);
         CPPUNIT_ASSERT(start.next == link1);
         CPPUNIT_ASSERT(link1->prev() == &start);
         CPPUNIT_ASSERT(link1->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link1);
         CPPUNIT_ASSERT(link3->next() == NULL);

         returnedData = link1->unlink();
         CPPUNIT_ASSERT(returnedData == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(link3->prev() == &start);
         CPPUNIT_ASSERT(link3->next() == NULL);
         CPPUNIT_ASSERT(start.prev == NULL);
         CPPUNIT_ASSERT(start.next == link3);

         returnedData = link3->unlink();
         CPPUNIT_ASSERT(returnedData == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(start.prev == NULL);
         CPPUNIT_ASSERT(start.next == NULL);
      }


   void testListAfter()
      {
         UtlChain list;

         UtlLink* link3 = UtlLink::listAfter(&list, NULL, &data3);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(list.next == link3);
         CPPUNIT_ASSERT(link3->prev() == NULL);
         CPPUNIT_ASSERT(link3->next() == NULL);
         CPPUNIT_ASSERT(list.prev == link3);

         UtlLink* link1 = UtlLink::listAfter(&list, NULL, &data1);

         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);
         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(list.next == link1);
         CPPUNIT_ASSERT(link1->prev() == NULL);
         CPPUNIT_ASSERT(link1->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link1);
         CPPUNIT_ASSERT(link3->next() == NULL);
         CPPUNIT_ASSERT(list.prev == link3);

         UtlLink* link2 = UtlLink::listAfter(&list, link1, &data2);

         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);
         CPPUNIT_ASSERT(link2->data   == &data2 && data2.getValue() == 2);
         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(list.next == link1);
         CPPUNIT_ASSERT(link1->prev() == NULL);
         CPPUNIT_ASSERT(link1->next() == link2);
         CPPUNIT_ASSERT(link2->prev() == link1);
         CPPUNIT_ASSERT(link2->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link2);
         CPPUNIT_ASSERT(link3->next() == NULL);
         CPPUNIT_ASSERT(list.prev == link3);

         void* returnedData;

         returnedData = link2->detachFrom(&list);
         CPPUNIT_ASSERT(returnedData == &data2 && data2.getValue() == 2);
         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);
         CPPUNIT_ASSERT(link1->data   == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(list.next == link1);
         CPPUNIT_ASSERT(link1->prev() == NULL);
         CPPUNIT_ASSERT(link1->next() == link3);
         CPPUNIT_ASSERT(link3->prev() == link1);
         CPPUNIT_ASSERT(link3->next() == NULL);
         CPPUNIT_ASSERT(list.prev == link3);

         returnedData = link1->detachFrom(&list);
         CPPUNIT_ASSERT(returnedData == &data1 && data1.getValue() == 1);

         CPPUNIT_ASSERT(link3->data   == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(list.next == link3);
         CPPUNIT_ASSERT(link3->prev() == NULL);
         CPPUNIT_ASSERT(link3->next() == NULL);
         CPPUNIT_ASSERT(list.prev == link3);

         returnedData = link3->detachFrom(&list);
         CPPUNIT_ASSERT(returnedData == &data3 && data3.getValue() == 3);

         CPPUNIT_ASSERT(list.prev == NULL);
         CPPUNIT_ASSERT(list.next == NULL);
      }



   void testLinkReuse()
      {
         UtlChain start;
         size_t startingPoolSize = totalAllocated();

         // chain on links until a new block is allocated
         int i;
         for (i = 0;
              startingPoolSize == totalAllocated();
              i++
              )
         {
            UtlLink::after(&start, &data1);
         }

         size_t peakPoolSize = totalAllocated(); // how many we had after that allocation

         // release all those links
         while (!start.isUnLinked())
         {
            start.head()->unlink();
         }

         // verify that the number of allocated links didn't change
         CPPUNIT_ASSERT(peakPoolSize == totalAllocated());

         // now repeatedly use more links, but no more than the peak usage, and free them again
         for (int iteration = 0; iteration < 1000; iteration++)
         {
            // fill the start list with the number of blocks it took to force an allocation
            for (unsigned i = 0; i < (peakPoolSize - startingPoolSize); i++)
            {
               UtlLink::after(&start, &data1);
            }
            char msg[1000];
            sprintf(msg,
                    "New allocations after filling in iteration %d\n"
                    "   startingPoolSize %zu\n"
                    "   peakPoolSize     %zu\n"
                    "   totalAllocated   %zu\n",
                    iteration, startingPoolSize, peakPoolSize, totalAllocated()
                    );
            CPPUNIT_ASSERT_MESSAGE(msg, peakPoolSize == totalAllocated());

            // release all those links
            while (!start.isUnLinked())
            {
               start.head()->unlink();
            }

            sprintf(msg,
                    "New allocations after release in iteration %d\n"
                    "   startingPoolSize %zu\n"
                    "   peakPoolSize     %zu\n"
                    "   totalAllocated   %zu\n",
                    iteration, startingPoolSize, peakPoolSize, totalAllocated()
                    );
            CPPUNIT_ASSERT_MESSAGE(msg, peakPoolSize == totalAllocated());
         }
      }


};

CPPUNIT_TEST_SUITE_REGISTRATION(UtlLinkTest);
