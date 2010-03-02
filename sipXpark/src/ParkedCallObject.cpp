//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include "ParkedCallObject.h"
#include <filereader/OrbitFileReader.h>
#include <net/Url.h>
#include <os/OsFS.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType ParkedCallObject::TYPE = "ParkedCallObject";

// STATIC VARIABLE INITIALIZATIONS

int ParkedCallObject::sNextSeqNo = 0;
const int ParkedCallObject::sSeqNoIncrement = 8;
const int ParkedCallObject::sSeqNoMask = 0x3FFFFFF8;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ParkedCallObject::ParkedCallObject(const UtlString& orbit,
                                   CallManager* callManager,
                                   const UtlString& callId,
                                   const UtlString& address,
                                   const UtlString& playFile,
                                   bool bPickup,
                                   OsMsgQ* listenerQ,
                                   const OsTime& lifetime,
                                   const OsTime& blindXferWait,
                                   const OsTime& keepAliveTime) :
   mSeqNo(sNextSeqNo),
   mpCallManager(callManager),
   mOriginalCallId(callId),
   mCurrentCallId(callId),
   mOriginalAddress(address),
   mCurrentAddress(address),
   mpPlayer(NULL),
   mFile(playFile),
   mOrbit(orbit),
   mbPickup(bPickup),
   mbEstablished(false),
   mTimeoutTimer(listenerQ, (void*)(mSeqNo + PARKER_TIMEOUT)),
   mMaximumTimer(listenerQ, (void*)(mSeqNo + MAXIMUM_TIMEOUT)),
   mTransferTimer(listenerQ, (void*)(mSeqNo + TRANSFER_TIMEOUT)),
   mKeepAliveTimer(listenerQ, (void*)(mSeqNo + KEEPALIVE_TIMEOUT)),
   // Create the OsQueuedEvent to handle DTMF events.
   // This would ordinarily be an allocated object, because
   // removeDtmfEvent will delete it asynchronously.
   // But we do not need to call removeDtmfEvent, as we let call
   // teardown remove the pointer to mDtmfEvent (without attempting to
   // delete it).  We know that call teardown happens first, becausse
   // we do not delete a ParkedCallObject before knowing that it is
   // torn down.
   mDtmfEvent(*listenerQ, (void*)(mSeqNo + DTMF)),
   mKeycode(OrbitData::NO_KEYCODE),
   mTransferInProgress(FALSE),
   mBlindXferWait(blindXferWait)
{
   OsDateTime::getCurTime(mParked);
   // Update sNextSeqNo.
   sNextSeqNo = (sNextSeqNo + sSeqNoIncrement) & sSeqNoMask;
   // Start the maximum timer.
   mMaximumTimer.oneshotAfter(lifetime);
   // Set the keepalive timer.
   mKeepAliveTimer.periodicEvery(keepAliveTime, keepAliveTime);
}

ParkedCallObject::~ParkedCallObject()
{
   // Stop the Keepalive timer
   mKeepAliveTimer.stop();

   // Terminate the audio player, if any.
   if (mpPlayer)
   {
      mpCallManager->destroyPlayer(MpPlayer::STREAM_PLAYER,
                                   mCurrentCallId,
                                   mpPlayer);
   }
   // Stop the escape timer and stop listening for DTMF.
   stopEscapeTimer();
}


const char* ParkedCallObject::getOriginalAddress()
{
   return mOriginalAddress.data();
}


void ParkedCallObject::setCurrentAddress(const UtlString& address)
{
   mCurrentAddress = address;
}


const char* ParkedCallObject::getCurrentAddress()
{
   return mCurrentAddress.data();
}


const char* ParkedCallObject::getOriginalCallId()
{
   return mOriginalCallId.data();
}

void ParkedCallObject::setCurrentCallId(const UtlString& callId)
{
   mCurrentCallId = callId;
}


const char* ParkedCallObject::getCurrentCallId()
{
   return mCurrentCallId.data();
}


const char* ParkedCallObject::getOrbit()
{
   return mOrbit.data();
}


void ParkedCallObject::getTimeParked(OsTime& parked)
{
   parked = mParked;
}


bool ParkedCallObject::isPickupCall()
{
   return mbPickup;
}


OsStatus ParkedCallObject::playAudio()
{
   OsStatus result = OS_SUCCESS;

   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::playAudio "
                 "CallId %s is requesting to play the audio file",
                 mCurrentCallId.data());

   // Create an audio player and queue up the audio to be played.
   mpCallManager->createPlayer(MpPlayer::STREAM_PLAYER,
                               mCurrentCallId,
                               mFile.data(),
                               STREAM_SOUND_REMOTE | STREAM_FORMAT_WAV,
                               &mpPlayer) ;

   if (mpPlayer == NULL)
   {
      OsSysLog::add(FAC_PARK, PRI_ERR,
                    "ParkedCallObject::playAudio "
                    "CallId %s: Failed to create player for '%s'",
                    mCurrentCallId.data(), mFile.data());
      return OS_FAILED;
   }

   mpPlayer->setLoopCount(-1);    // Play forever.

   if (mpPlayer->realize(TRUE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_PARK, PRI_ERR,
                    "ParkedCallObject::playAudio - CallId %s: Failed to realize player for '%s'",
                    mCurrentCallId.data(), mFile.data());
      return OS_FAILED;
   }

   if (mpPlayer->prefetch(TRUE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_PARK, PRI_ERR,
                    "ParkedCallObject::playAudio - CallId %s: Failed to prefetch player",
                    mCurrentCallId.data());
      return OS_FAILED;
   }

   if (mpPlayer->play(FALSE) != OS_SUCCESS)
   {
      OsSysLog::add(FAC_PARK, PRI_ERR,
                    "ParkedCallObject::playAudio - CallId %s: Failed to play",
                    mCurrentCallId.data());
      return OS_FAILED;
   }
   OsSysLog::add(FAC_PARK, PRI_DEBUG, "ParkedCallObject::playAudio - Successful");

   return result;
}


// Activate the escape mechanisms, if the right conditions are present.
// One is the time-out timer, which if it expires, will transfer
// the call back to the user that parked it.
// The other is the escape keycode, which lets the user transfer the call back.
// Neither mechanism is activated if there is no parker URI to transfer back
// to.  Both need appropriate configuration items in the orbits.xml file
// to be activated.
void ParkedCallObject::startEscapeTimer(UtlString& parker,
                                        int timeout,
                                        int keycode)
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::startEscapeTimer callId = '%s', "
                 "parker = '%s', timeout = %d, keycode = %d",
                 mCurrentCallId.data(), parker.data(), timeout, keycode);

   // First, check that there is a parker URI.  If not, none of these
   // mechanisms can function.
   if (parker.isNull())
   {
      return;
   }
   // Here, we can insert further validation, such as that the parker URI
   // is local.

   // Save the parker URI.
   mParker = parker;

   if (timeout != OrbitData::NO_TIMEOUT)
   {
      // Set the timeout timer.
      OsTime timeoutOsTime(timeout, 0);
      // Use a periodic timer, so if the transfer generated by one timeout
      // fails, we will try again later.
      mTimeoutTimer.periodicEvery(timeoutOsTime, timeoutOsTime);
   }
   // Remember the keycode for escaping, if any.
   mKeycode = keycode;
   if (mKeycode != OrbitData::NO_KEYCODE)
   {
      // Register the DTMF listener.
      // The "interdigit timeout" time of 1 is just a guess.
      // Enable keyup events, as those are the ones we will act on.
      mpCallManager->enableDtmfEvent(mCurrentCallId.data(), 1,
                                     &mDtmfEvent, false);
   }
}


// Stop the parking escape mechanisms.
void ParkedCallObject::stopEscapeTimer()
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::stopEscapeTimer callId = '%s'",
                 mCurrentCallId.data());
   mTimeoutTimer.stop();
   if (mKeycode != OrbitData::NO_KEYCODE)
   {
      // We can't use removeDtmfEvent() here, because it would try to
      // free mDtmfEvent.
      mpCallManager->disableDtmfEvent(mCurrentCallId.data(),
                                      &mDtmfEvent);
      mKeycode = OrbitData::NO_KEYCODE;
   }
}


// Do a blind transfer of this call to mParker.
void ParkedCallObject::startBlindTransfer()
{
   if (!mTransferInProgress)
   {
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "ParkedCallObject::startBlindTransfer "
                    "starting transfer "
                    "callId = '%s', parker = '%s'",
                    mCurrentCallId.data(), mParker.data());
      // Start the timer to detect if the blind transfer fails.
      markTransfer(mBlindXferWait);
      // Put the parked caller on hold while we attempt the transfer.
      // This gives the caller feedback on the progress of his transfer attempt.
      // We work around the bug in Polycom 2.0.0 by only transferring on
      // key-up events, so the key is never down when we send the phone
      // a re-INVITE.
      mpCallManager->transfer_blind(mCurrentCallId,
                                    mParker,
                                    NULL, NULL,
                                    TRUE);
   }
   else
   {
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "ParkedCallObject::startBlindTransfer "
                    "transfer already in progress "
                    "callId = '%s', parker = '%s'",
                    mCurrentCallId.data(), mParker.data());
   }
}


// Start the transfer deadman timer, and set mTransferInProgress.
void ParkedCallObject::markTransfer(const OsTime &timeOut)
{
   // Mark that a transfer is in progress (and so the call is not
   // available for other transfer attempts).
   mTransferInProgress = TRUE;
   // Start the timer to detect failed transfer attempts.
   mTransferTimer.oneshotAfter(timeOut);
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::markTransfer "
                 "transfer timer started "
                 "callId = '%s', time = %d.%06d",
                 mCurrentCallId.data(), (int) timeOut.seconds(),
                 (int) timeOut.usecs());
}


// Signal that a transfer attempt for a call has ended.
// The transfer may or may not be successful.  (If it was successful,
// one of the UAs will terminate this call soon.)  Re-enable initiating
// transfers.
void ParkedCallObject::clearTransfer()
{
   // Stop the deadman timer.
   mTransferTimer.stop();
   mTransferInProgress = FALSE;
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::clearTransfer transfer cleared "
                 "callId = '%s'",
                 mCurrentCallId.data());
}

// Send a keep alive signal back to the caller.
void ParkedCallObject::sendKeepAlive(const char * mohUserPart)
{
   // See if this is a music-on-hold call or an orbit call

   if (mOrbit == mohUserPart)
   {
      // Send a keep alive signal back to the caller. Use OPTIONS
      mpCallManager->sendKeepAlive(mCurrentCallId, TRUE);
   }
   else
   {
      // Send a keep alive signal back to the caller. Use Re-INVITEs
      mpCallManager->sendKeepAlive(mCurrentCallId, FALSE);
   }
}

// Process a DTMF keycode for this call.
void ParkedCallObject::keypress(int keycode)
{
   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "ParkedCallObject::keypress "
                 "callId = '%s', parker = '%s', keycode = %d",
                 mCurrentCallId.data(), mParker.data(), keycode);
   // Must test if the keypress is to cause a transfer.
   if (mKeycode != OrbitData::NO_KEYCODE
       && keycode == mKeycode
       && !mParker.isNull())
   {
      // Call startBlindTransfer, which will check if a transfer is
      // already in progress, and if not, start a blind transfer.
      startBlindTransfer();
   }
}


/**
 * Get the ContainableType.
 */
UtlContainableType ParkedCallObject::getContainableType() const
{
   return TYPE;
}
