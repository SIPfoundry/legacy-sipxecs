//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDLine_h_
#define _ACDLine_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/sipXtapi.h>
#include <utl/UtlString.h>
#include <utl/UtlContainable.h>
#include <net/SipDialogEvent.h>
#include <net/ProvisioningAttrList.h>
#include "ACDCall.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ACDCallManager;
class ACDLineManager;

// ACDLine object is an abstraction of a SIP UA line presence.  Through
// its public interface, it is possible to create, modify, monitor and
// destroy line presences for the associated ACDCallManager UA.

class ACDLine : public UtlContainable {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   ACDLine(ACDLineManager* pAcdLineManager,
           SIPX_LINE       lineHandle,
           const char*     pLineUriString,
           const char*     pName,
           const char*     pExtension,
           bool            trunkMode,
           bool            publishLinePresence,
           const char*     pAcdQueue);

   /**
    * Destructor
    */
   virtual ~ACDLine();

/* ============================ MANIPULATORS ============================== */

   OsStatus publishCallState(ACDCall* pCallRef, ACDCall::eCallState state);

   bool addCall(ACDCall* pCallRef);

   void deleteCall(ACDCall* pCallRef);

   void setAttributes(ProvisioningAttrList& rRequestAttributes);

/* ============================ ACCESSORS ================================= */

   SIPX_LINE getLineHandle(void);

   UtlString* getUriString(void);

   void getAttributes(ProvisioningAttrList& rRequestAttributes, ProvisioningAttrList*& prResponse);

   virtual unsigned hash() const;

   virtual UtlContainableType getContainableType() const;

   UtlString getAcdQueue() { return mAcdQueue;};

   ACDLineManager* getAcdLineManager(){return mpAcdLineManager;};
/* ============================ INQUIRY =================================== */

   bool isLineBusy(void);

   virtual int compareTo(UtlContainable const *) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static const UtlContainableType TYPE; // Class type used for runtime checking

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex         mLock;                    // Lock used for atomic access
   Url             mUri;                     // The AOR for this line
   UtlString       mUriString;               // The AOR for this line
   UtlString       mName;                    // The discriptive name of this line
   UtlString       mExtension;               // The extension number for this line
   bool            mTrunkMode;               // Does this line operate as a trunk
   bool            mPublishLinePresence;     // If true, publish line presence via dialog events
   UtlString       mAcdQueue;                // The ACDQueue that this line is assigned to
   SIPX_LINE       mhLineHandle;             // The sipXtapi handle for this line
   ACDLineManager* mpAcdLineManager;         // Reference to the parent LineManager object
   SIPX_INST       mhAcdCallManagerHandle;   // The sipXtapi handle for the UA
   ACDCallManager* mpAcdCallManager;         // Reference to the parent CallManager object
   SipDialogEvent* mpDialogEventPackage;     // Dialog Event Package for this line presence
   unsigned long   mDialogId;                // Incremental dialog ID counter
   SIPX_PUB        mhPublisherHandle;        // The sipXtapi handle for the associated dialog publisher
   UtlString       mDialogPDU;               // Storage for dialog PDU
   ssize_t         mDialogPDULength;         // Length, in bytes, of the dialog PDU
   bool            mLineBusy;                // Used by non-trunk line to indicate if busy with call
   UtlHashMap      mCalls;                   // Number of calls on this line
};

#endif  // _ACDLine_h_
