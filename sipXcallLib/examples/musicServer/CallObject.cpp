//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <CallObject.h>
#include <net/Url.h>
#include <os/OsFS.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CallObject::CallObject(CallManager* callManager, UtlString callId, UtlString playFile)
{
   mpCallManager = callManager;
   mCallId = callId;

   mpPlayer = NULL;
   mFile = playFile;
}

CallObject::~CallObject()
{
   mpCallManager = NULL;
}


OsStatus CallObject::playAudio()
{
   OsStatus result = OS_SUCCESS;
   char szUrl[128] ;

   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "CallId %s is requesting for playing the wavefile ...\n", mCallId.data());

   mpCallManager->createPlayer(mCallId, &mpPlayer) ;

   if (mpPlayer == NULL)
   {
      return OS_FAILED;
   }

   // Send a sequence of prompts to the player
   for (int i=0; i<3; i++)
   {
      sprintf(szUrl, "Playlist +file://%s", mFile.data());

      Url url(szUrl) ;
      mpPlayer->add(url, STREAM_SOUND_REMOTE | STREAM_FORMAT_WAV) ;
   }

   if (mpPlayer->realize(TRUE) != OS_SUCCESS)
   {
      cleanUp();
      return OS_FAILED;
   }

   if (mpPlayer->prefetch(TRUE) != OS_SUCCESS)
   {
      cleanUp();
      return OS_FAILED;
   }

   if (mpPlayer->play(FALSE) != OS_SUCCESS)
   {
      cleanUp();
      return OS_FAILED;
   }

   return result;
}


void CallObject::cleanUp()
{
   if (mpPlayer)
      mpCallManager->destroyPlayer(mCallId, mpPlayer);

   mpPlayer = NULL;
}
