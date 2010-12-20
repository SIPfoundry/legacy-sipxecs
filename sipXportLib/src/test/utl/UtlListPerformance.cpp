//////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "assert.h"

// APPLICATION INCLUDES

#include "utl/UtlString.h"
#include "utl/UtlSList.h"
#include "utl/UtlSListIterator.h"
#include "os/OsTask.h"
#include "os/OsTimeLog.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
int externalForSideEffects;

// CONSTANTS
// comparison base values
#include "UtlPerformanceStrings.h"
#define NUM_THREADS 5

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
void doListOperations();

void appendCountItems(UtlSList& list, size_t itemsToAdd);

void getCountItems(UtlSList& list, size_t itemsToPop);

OsTimeLog timer((NUM_THREADS + 1) * 2);

class doTestThread : public OsTask
{
public:
   int run(void* taskArg)
      {
         char mynum[3];
         sprintf(mynum, "%02d", getUserData());
         UtlString startMsg("  start  ");
         UtlString finishMsg("  finish ");
         startMsg.append(mynum);
         finishMsg.append(mynum);

         timer.addEvent(startMsg.data());
         doListOperations();
         timer.addEvent(finishMsg.data());
         return 0;
      }

   UtlBoolean waitUntilShutDown()
      {
         this->OsTask::waitUntilShutDown();
         return TRUE;
      }
};

int main()
{
   doTestThread* threads[NUM_THREADS];
   int n;

   UtlSList dummy;

   for (n = 0; n < NUM_THREADS; n++)
   {
      threads[n]   = new doTestThread;
      threads[n]->setUserData(n);
   }

   timer.addEvent("All Start");

   for (n = 0; n < NUM_THREADS; n++)
   {
      threads[n]->start();
   }

   for (n = 0; n < NUM_THREADS; n++)
   {
      threads[n]->waitUntilShutDown();
   }

   timer.addEvent("Done");

   timer.dumpLog();

   return 0;
}


void doListOperations()
{
   UtlSList testList;

   // fill the list
   appendCountItems(testList, NUM_PERFORMANCE_STRINGS);

   // take the first half off the front
   if (!testList.isEmpty())
   {
      getCountItems(testList, NUM_PERFORMANCE_STRINGS / 2);
   }

   // take the rest off the end by reference
   if (!testList.isEmpty())
   {
      UtlContainable* lastItem = testList.last();
      delete dynamic_cast<UtlString*>(testList.removeReference(lastItem));
   }

   // fill the list
   appendCountItems(testList, NUM_PERFORMANCE_STRINGS);

   // search the list for each item by value
   UtlString target;
   int targetIndex;
   for (targetIndex = 0; targetIndex < NUM_PERFORMANCE_STRINGS; targetIndex += 1)
   {
      target = string[targetIndex];
      UtlString* found = dynamic_cast<UtlString*>(testList.find(&target));
      if (found)
      {
         externalForSideEffects = found->length();
      }
   }

   // get the object in the middle of the list by index, and remove it by value
   while(!testList.isEmpty())
   {
      int numberLeft = testList.entries();
      UtlString* middle = dynamic_cast<UtlString*>(testList.at((numberLeft / 2)));
      delete dynamic_cast<UtlString*>(testList.remove(middle));
   }

   // fill the list
   appendCountItems(testList, NUM_PERFORMANCE_STRINGS);

   // iterate over each item in the list
   UtlSListIterator iterate(testList);
   UtlString* item;
   while ((item = dynamic_cast<UtlString*>(iterate())))
   {
      externalForSideEffects = item->length();
      delete item;
   }
}


void appendCountItems(UtlSList& list, size_t itemsToAdd)
{
   assert(itemsToAdd);
   size_t item;

   for (item = 0; item < itemsToAdd; item++)
   {
      list.append(new UtlString(string[item]));
   }
}


void getCountItems(UtlSList& list, size_t itemsToPop)
{
   assert(itemsToPop);

   for (; !list.isEmpty() && itemsToPop; itemsToPop--)
   {
      delete dynamic_cast<UtlString*>(list.get());
   }
}
