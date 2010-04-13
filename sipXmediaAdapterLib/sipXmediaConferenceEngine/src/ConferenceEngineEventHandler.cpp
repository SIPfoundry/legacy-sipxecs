// $Id$
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include "include/ConferenceEngineEventHandler.h"
#include "include/ConferenceEngineMediaInterface.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ConferenceEngineEventHandler::ConferenceEngineEventHandler()
{

}


// Destructor
ConferenceEngineEventHandler::~ConferenceEngineEventHandler()
{

}

/* ============================ MANIPULATORS ============================== */



/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */



/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

void ConferenceEngineEventHandler::ReceivedOutbandDTMF(void* anyPtr, int channel, int value)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "ConferenceEngineEventHandler::ReceivedOutbandDTMF receiving a DTMF event %d on channel %d",
                  value, channel);

    ConferenceEngineMediaInterface* mediaInterface = (ConferenceEngineMediaInterface*) anyPtr;

    OsNotification* notifier = NULL;
    notifier = mediaInterface->getDTMFNotifier(channel);

    OsStatus ret;
    if (NULL != notifier)
    {
        ret = notifier->signal(0x80000000 |
                               (0x3fff0000 & (value << 16)) |
                               (800 & 0xffff));
        if (OS_SUCCESS != ret)
        {
            if (OS_ALREADY_SIGNALED == ret)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              "ConferenceEngineEventHandler::ReceivedOutbandDTMF Signal Stop returned OS_ALREADY_SIGNALED");

            }
            else
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              "ConferenceEngineEventHandler::ReceivedOutbandDTMF Signal Stop returned %d", ret);
            }
        }
    }
}

void ConferenceEngineEventHandler::ReceivedInbandDTMF(void* anyPtr, int channel, int tone)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "ConferenceEngineEventHandler::ReceivedInbandDTMF receiving a DTMF event %d on channel %d",
                  tone, channel);

    ConferenceEngineMediaInterface* mediaInterface = (ConferenceEngineMediaInterface*) anyPtr;

    OsNotification* notifier = NULL;
    notifier = mediaInterface->getDTMFNotifier(channel);

    OsStatus ret;
    if (NULL != notifier)
    {
        ret = notifier->signal(0x80000000 |
                               (0x3fff0000 & (tone << 16)) |
                               (1000 & 0xffff));
        if (OS_SUCCESS != ret)
        {
            if (OS_ALREADY_SIGNALED == ret)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              "ConferenceEngineEventHandler::ReceivedInbandDTMF Signal Stop returned OS_ALREADY_SIGNALED");

            }
            else
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              "ConferenceEngineEventHandler::ReceivedInbandDTMF Signal Stop returned %d", ret);
            }
        }
    }
}

void ConferenceEngineEventHandler::PlayedFileEnded(void* anyPtr, int channel, const char* fileName)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "ConferenceEngineEventHandler::PlayedFileEnded playFile %s done on channel %d",
                  fileName, channel);

    ConferenceEngineMediaInterface* mediaInterface = (ConferenceEngineMediaInterface*) anyPtr;

    OsNotification* notifier = NULL;
    notifier = mediaInterface->getPlayNotifier(channel);

    OsStatus ret;
    if (NULL != notifier)
    {
    }
}

void ConferenceEngineEventHandler::EventMessage(void* anyPtr, GIPS_EVENT_MESSAGE type, const char* str)
{
    switch (type)
    {
    case GIPS_INFORMATION:
        OsSysLog::add(FAC_MP, PRI_INFO,
                      "ConferenceEngineEventHandler::EventMessage receiving event message %s",
                      str);
        break;
    case GIPS_WARNING:
        OsSysLog::add(FAC_MP, PRI_WARNING,
                      "ConferenceEngineEventHandler::EventMessage receiving event message %s",
                      str);
        break;
    case GIPS_ERROR:
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineEventHandler::EventMessage receiving event message %s",
                      str);
        break;
    case GIPS_DEBUG:
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "ConferenceEngineEventHandler::EventMessage receiving event message %s",
                      str);
        break;
    }
}
