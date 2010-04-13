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

#include "sipxportlib-buildstamp.h"
#include "utl/UtlInt.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlHashMapIterator.h"
#include "os/OsTask.h"
#include "os/OsTimeLog.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
int externalForSideEffects;

// CONSTANTS
// comparison base values
#define NUM_THREADS 5

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
void doHashMapOperations();

#include "UtlPerformanceStrings.h"

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
         doHashMapOperations();
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

   setupStrings();

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

   osPrintf("UtlHashMap Performance v=%s %s:\n",
            SipXportlibVersion, SipXportlibBuildStamp
            );
   timer.dumpLog();

   return 0;
}


void doHashMapOperations()
{
   UtlHashMap testHash;
   UtlInt* intValue;
   size_t item;

   // fill the hash table
   for (item = 0; item < NUM_PERFORMANCE_STRINGS; item++)
   {
      UtlInt* newValue = new UtlInt(item);
      testHash.insertKeyAndValue(&string[item], newValue);
      intValue = dynamic_cast<UtlInt*>(testHash.findValue(&string[item]));
      externalForSideEffects = (intValue == newValue);
   }

   // take the first half out by value
   for (item = 0; item < NUM_PERFORMANCE_STRINGS/2; item++)
   {
      UtlString key(string[item]); // make a copy so that no reference matching will work
      UtlContainable* foundValue;
      testHash.removeKeyAndValue(&key, foundValue);
      delete foundValue;
   }

   // take the rest out by reference
   for (; item < NUM_PERFORMANCE_STRINGS; item++)
   {
      intValue = dynamic_cast<UtlInt*>(testHash.removeReference(&string[item]));
      delete intValue;
   }

   // fill the list again
   for (item = 0; item < NUM_PERFORMANCE_STRINGS; item++)
   {
      testHash.insertKeyAndValue(&string[item], &string[item]);
      externalForSideEffects = testHash.entries();
   }

   // iterate over each item in the hash
   UtlHashMapIterator iterate(testHash);
   UtlString* strValue = NULL;
   UtlString* entry;
   while ((entry = dynamic_cast<UtlString*>(iterate())))
   {
      strValue = dynamic_cast<UtlString*>(testHash.findValue(entry));
   }
   externalForSideEffects = (strValue == &string[0]); // meaningless, just prevents optimization

   testHash.removeAll();
}
