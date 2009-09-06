//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _ParkedCallObject_h_
#define _ParkedCallObject_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cp/CallManager.h>
#include <mp/MpStreamPlayer.h>
#include <os/OsDateTime.h>
#include <utl/UtlSList.h>
#include <os/OsTimer.h>
#include <os/OsMsgQ.h>
#include <utl/UtlContainableAtomic.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


//: Object to describe and control a parked call.
//  All methods are executed by the thread of the owning OrbitListener.
class ParkedCallObject : public UtlContainableAtomic
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    // Enum to differentiate NOTIFY messages.
    // Max value must be less than sSeqNoIncrement.
    enum notifyCodes
       {
          // A DTMF event was received on this dialog.
          DTMF,
          // The timeout was reached for transferring this call back to its parker.
          PARKER_TIMEOUT,
          // The timeout was reached for the maximum lifetime of a parked call.
          MAXIMUM_TIMEOUT,
          // The current transfer operation has timed out.
          TRANSFER_TIMEOUT,
          // The keepalive timer
          KEEPALIVE_TIMEOUT
       };

/* ============================ CREATORS ================================== */

   ParkedCallObject(const UtlString& orbit,
                    CallManager* callManager,
                    const UtlString& callId,
                    const UtlString& address,
                    const UtlString& playFile,
                    bool bPickup,
                    OsMsgQ* listenerQ,
                    const OsTime& lifetime,
                    const OsTime& blindXferWait,
                    const OsTime& keepAliveTime);
   /// Destroy the ParkedCallObject.
   // Also calls stopEscapeTimer().
   ~ParkedCallObject();

   // Accessor/modifier functions for these fields.
   // Note that most accessor functions return a const char*, which
   // should not be stored longer than any operation which might
   // change the Park Server state.

   const char* getOriginalAddress();
   void setCurrentAddress(const UtlString& address);
   const char* getCurrentAddress();

   const char* getOriginalCallId();
   void setCurrentCallId(const UtlString& callId);
   const char* getCurrentCallId();

   const char* getOrbit();
   void getTimeParked(OsTime& parked);
   bool isPickupCall();

   OsStatus playAudio();

   // Set up the "escape from parking orbit" mechanisms:
   // If a parker URI and timeout are supplied, set a timer to trigger
   // a transfer to the parker.
   // If a parker URI and DTMF code are supplied, set a DTMF listener
   // to allow the user to force a transfer to the parker.
   void startEscapeTimer(UtlString& parker,
                         ///< URI that parked this call, or "".
                         int timeout,
                         ///< seconds for timeout, or OrbitData::NO_TIMEOUT
                         int keycode
                         /**< RFC 2833 keycode to escape from park, or
                          *   OrbitData::NO_KEYCODE for none.
                          */
      );

   // Stop the time-out timer.

   // The escape mechanisms are cancelled automatically by the
   // ParkedCallObject::~, and since their notifications are done via
   // messages to the OrbitListener that contain only mSeqNo, race
   // conditions are not a problem.  But when a dialog is replaced by
   // another dialog, we need to stop the escape timer on the ParkedCallObject
   // for the first dialog.
   void stopEscapeTimer();

   // Initiate a blind transfer of this call to mParker.
   void startBlindTransfer();

   // Start the transfer deadman timer, and set mTransferInProgress.
   void markTransfer(const OsTime &timeOut);

   // Signal that a transfer attempt for a call has ended.
   void clearTransfer();

   // Send a keep alive signal back to the caller.
   void sendKeepAlive(const char * mohUserPart);

   // Determine if a transfer attempt is in progress.
   UtlBoolean transferInProgress();

   // Process a DTMF keycode.
   void keypress(int keycode);

   // Split a userData value into the seqNo and "enum notifyCodes".
   static void splitUserData(int userData, int& seqNo, enum notifyCodes& type)
      {
         seqNo = userData & sSeqNoMask;
         type = (enum notifyCodes) (userData & ~sSeqNoMask);
      };

   // Get the seqNo.
   int getSeqNo()
      {
         return mSeqNo;
      };

   // Set the CALL_ESTABLISHED flag.
   void setEstablished();

   // Get the CALL_ESTABLISHED flag.
   bool getEstablished();

/* ============================ INQUIRY =================================== */

    virtual UtlContainableType getContainableType() const;

    static const UtlContainableType TYPE;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    // Sequence number of this ParkedCallObject.
    int mSeqNo;
    // Next seqNo to be assigned.
    static int sNextSeqNo;
    // Amount to increment successive seqNo's.
    // Must be lowest 1 bit of seqNoMask.
    static const int sSeqNoIncrement;
    // Mask to cause seqNo's to wrap around.
    static const int sSeqNoMask;

    CallManager* mpCallManager;

    // Because the Call Manager reports events for later legs of a call
    // that was replaced by another call using the call-Id of the original
    // leg, we must keep track of both the original and current call-Id
    // for a single logical call.
    UtlString mOriginalCallId, mCurrentCallId;
    // Call Manager events for the different legs are distinguished by
    // the address-of-the-other-end field of its messages.
    UtlString mOriginalAddress, mCurrentAddress;

    MpStreamPlayer* mpPlayer;
    UtlString mFile;
    // The orbit number for this call.
    UtlString mOrbit;

    bool mbPickup;             // Call is a retrieval call

    bool mbEstablished;         /**< CALL_ESTABLISHED has been seen
                                 *   for this call. */
    OsTime mParked;             /**< When the ParkedCallObject was created,
                                 *   which is when the call was parked.
                                 */

    // Members to support the transfer back to parker feature.
    UtlString mParker;          ///< The URI of the user that parked the call.
    OsTimer mTimeoutTimer;      ///< OsTimer to trigger the timeout.

    // Support the maximum lifetime feature.
    OsTimer mMaximumTimer;      ///< OsTimer to trigger the maximum timeout.

    // Deadman timer for transfer attempts.
    OsTimer mTransferTimer;     ///< OsTimer to detect failed transfer attempts.

    // Keepalive timer.
    OsTimer mKeepAliveTimer;     ///< OsTimer to trigger periodic keepalives.

    // Support for processing DTMF events.
    OsQueuedEvent mDtmfEvent;
    int mKeycode;               /**< keycode to transfer back, or
                                 *   OrbitData::NO_KEYCODE */
    /// Set to TRUE if a transfer to the parker is in progress.
    //  Used to ensure that a transfer is not started if one is already
    //  started.
    UtlBoolean mTransferInProgress;

    // The time to allow for a blind transfer.
    OsTime mBlindXferWait;
};

/* ============================ INLINE METHODS ============================ */

// Set the CALL_ESTABLISHED flag.
inline void ParkedCallObject::setEstablished()
{
   mbEstablished = true;
}

// Get the CALL_ESTABLISHED flag.
inline bool ParkedCallObject::getEstablished()
{
   return mbEstablished;
}

// Return TRUE if a transfer back to parker is in progress.
inline UtlBoolean ParkedCallObject::transferInProgress()
{
   return mTransferInProgress;
}

#endif  // _ParkedCallObject_h_
