// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#if defined(_WIN32)
#   include <io.h>
#elif defined(_VXWORKS)
#   include <unistd.h>
#   include <dirent.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#   include <stdlib.h>
#   define O_BINARY 0 // There is no notion of a "not binary" file under POSIX,
                      // so we just set O_BINARY used below to no bits in the mask.
#else
#   error Unsupported target platform.
#endif
#include <fcntl.h> 

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif //TEST

// APPLICATION INCLUDES
#include "IvrDtmfListener.h"
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>
#include <cp/CallManager.h>
#include <net/Url.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
IvrDtmfListener::IvrDtmfListener(VXIplatform *platform, const UtlString& name) : 
TaoAdaptor(name)
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

   mpPlatform = platform;
}

// Copy constructor
IvrDtmfListener::IvrDtmfListener(const IvrDtmfListener& rIvrDtmfListener)
{
   mpPlatform = rIvrDtmfListener.mpPlatform;
}

// Destructor
IvrDtmfListener::~IvrDtmfListener()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
IvrDtmfListener& 
IvrDtmfListener::operator=(const IvrDtmfListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   mpPlatform = rhs.mpPlatform;
   return *this;
}

UtlBoolean IvrDtmfListener::handleMessage(OsMsg& rMsg)
{
/* There is nothing to do for now */
/*
  if(rMsg.getMsgSubType()== TaoMessage::EVENT)
  {
  TaoMessage* taoMessage = (TaoMessage*)&rMsg;

  unsigned char taoEventId = taoMessage->getTaoObjHandle();
  UtlString argList(taoMessage->getArgList());

  osPrintf("===>\nMessage type: %d args: %s\n\n",
  taoEventId, argList.data());
  TaoString arg(argList, TAOMESSAGE_DELIMITER);
  int argc = arg.getCnt();
  for(int argIndex = 0; argIndex < argc; argIndex++)
  {
  osPrintf("\targ[%d]=\"%s\"\n", argIndex,
  arg[argIndex]);
  }

  UtlBoolean localConnection = atoi(arg[3]);
  if(taoEventId == TaoMessage::BUTTON_PRESS)
  {
  osPrintf("IVR: answering callId: %s address: %s\n",
  arg[0], arg[1]);
  }
  }
*/
   return(TRUE);
}

void IvrDtmfListener::setPlatform(VXIplatform *platform)
{
   mpPlatform = platform;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool IvrDtmfListener::sIsTested = false;

// Test this class by running all of its assertion tests
void IvrDtmfListener::test()
{
   UtlMemCheck* pMemCheck = 0;
   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   testCreators();
   testManipulators();
   testAccessors();
   testInquiry();

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the creators (and destructor) methods for the class
void IvrDtmfListener::testCreators()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // test the default constructor (if implemented)
   // test the copy constructor (if implemented)
   // test other constructors (if implemented)
   //    if a constructor parameter is used to set information in an ancestor
   //       class, then verify it gets set correctly (i.e., via ancestor
   //       class accessor method.
   // test the destructor
   //    if the class contains member pointer variables, verify that the 
   //    pointers are getting scrubbed.

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the manipulator methods
void IvrDtmfListener::testManipulators()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the accessor methods for the class
void IvrDtmfListener::testAccessors()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // body of the test goes here

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

// Test the inquiry methods for the class
void IvrDtmfListener::testInquiry()
{
   UtlMemCheck* pMemCheck  = 0;

   pMemCheck = new UtlMemCheck();      // checkpoint for memory leak check

   // body of the test goes here

   assert(pMemCheck->delta() == 0);    // check for memory leak
   delete pMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */

