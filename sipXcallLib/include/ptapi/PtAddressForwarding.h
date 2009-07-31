//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtAddressForwarding_h_
#define _PtAddressForwarding_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "ptapi/PtDefs.h"
#include "tao/TaoAddressAdaptor.h"
#include "cp/CpCallManager.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The PtAddressForwarding class represents a forwarding instruction.
// This instruction indicates how the phone system should forward
// incoming telephone calls to a specific address. The attributes
// of a forwarding instruction are:
// <p>
// <ul><li>
// The forwarding instruction's <b>type</b> tells the phone
// system when to forward the call. Currently, three types are
// supported: always forward incoming calls, forward incoming calls
// when the address is busy, and forward incoming calls when no
// one answers.</li>
// <br>
// <li>
// The forwarding instruction's <b>filter</b> identifies the set
// of incoming calls to which it applies. A forwarding instruction
// can apply to all calls, to external calls only, to internal calls
// only, or to calls from a specific calling address. </li>
// </ul>

class PtAddressForwarding
{
friend class PtAddress;
friend class CpCallManager;
friend TaoStatus TaoAddressAdaptor::addressSetForwarding(TaoMessage& rMsg);

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum ForwardingType
   {
          FORWARD_UNCONDITIONALLY = 1,
      FORWARD_ON_BUSY         = 2,
      FORWARD_ON_NOANSWER     = 3

   };
   //!enumcode: FORWARD_UNCONDITIONALLY - Forward calls unconditionally
   //!enumcode: FORWARD_ON_BUSY - Forward calls on busy
   //!enumcode: FORWARD_ON_NOANSWER - Forward calls on no answer

   enum FilterType
   {
      ALL_CALLS        = 1,
          INTERNAL_CALLS   = 2,
      EXTERNAL_CALLS   = 3,
      SPECIFIC_ADDRESS = 4
   };
   //!enumcode: ALL_CALLS - Apply forwarding instruction to all incoming calls
   //!enumcode: INTERNAL_CALLS - Apply instruction to calls originating within the provider's domain
   //!enumcode: EXTERNAL_CALLS - Apply instruction to calls originating from outside the provider's domain
   //!enumcode: SPECIFIC_ADDRESS - Apply instruction to calls originating from a specific address

/* ============================ CREATORS ================================== */

   PtAddressForwarding();
     //:Default constructor

   PtAddressForwarding(const char* destinationURL,
                       int type=FORWARD_UNCONDITIONALLY,
                                           int noAnswerTimeout = 0);
     //:Constructor variant #1
     // Creates a forwarding instruction that forwards all calls of the
     // indicated type to the <i>destinationURL</i> address.
     //!param: destinationURL - Destination address URL for the call forwarding operation
     //!param: type - Forwarding instruction type
     //!param: noAnswerTimeout - Timeout value for forwarding on no answer default 0 means 24 seconds
     // Returns a newly created PtAddressForwarding object

   PtAddressForwarding(const char* destinationURL, int type,
                       PtBoolean internalCalls, int noAnswerTimeout);
     //:Constructor variant #2
     // Creates a forwarding instruction that forwards calls of the
     // indicated type to the <i>destinationURL</i> address.  Depending
     // on the value of the <i>internalCalls</i> flag, this instruction
     // will affect either just internal or just external calls.
     //!param: destinationURL - Destination address URL for the call forwarding operation
     //!param: type - Forwarding instruction type
     //!param: internalCalls - If TRUE, forward only internal calls, otherwise forward only external calls
     //!param: noAnswerTimeout - Timeout value for forwarding on no answer
     // Returns a newly created PtAddressForwarding object

   PtAddressForwarding(const char* destinationURL, int type, const char* callerURL, int noAnswerTimeout);
     //:Constructor variant #3
     // Creates a forwarding instruction that applies only to incoming
     // calls from the indicated <i>callerURL</i>.
     //!param: destinationURL - Destination address URL for the call forwarding operation
     //!param: type - Forwarding instruction type
     //!param: callerURL - The URL for the incoming caller address that this forward operation affects
     //!param: noAnswerTimeout - Timeout value for forwarding on no answer
     // Returns a newly created PtAddressForwarding object

   PtAddressForwarding(const char* destinationURL, int type,
                       int filterType, const char* callerURL, int noAnswerTimeout);
     //:Constructor variant #4
     // Creates a forwarding instruction that forwards calls of the
     // indicated type to the <i>destinationURL</i> address.
     //!param: destinationURL - Destination address URL for the call forwarding operation
     //!param: type - Forwarding instruction type
     //!param: filterType - Forwarding filter type
     //!param: callerURL - The URL for the incoming caller address that this forward operation affects
     //!param: noAnswerTimeout - Timeout value for forwarding on no answer
     // Returns a newly created PtAddressForwarding object

   PtAddressForwarding(const PtAddressForwarding& rPtAddressForwarding);
     //:Copy constructor

   virtual
   ~PtAddressForwarding();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtAddressForwarding& operator=(const PtAddressForwarding& rhs);
     //:Assignment operator

   PtBoolean operator==(const PtAddressForwarding &rhs) const;
     //:equal operator

/* ============================ ACCESSORS ================================= */

   PtStatus getDestinationAddress(char* address, int len);
     //:Returns the destination URL associated with this forwarding
     //:instruction.

   PtStatus getFilter(int& filterType);
     //:Returns the filter type of this forwarding instruction.

   PtStatus getSpecificCaller(char* address, int len);
     //:Returns the specific incoming caller address associated with this
     //:forwarding instruction.
     // If the filter type for this forwarding instruction is
     // <i>SPECIFIC_ADDRESS</i>, then this method returns the
     // calling address URL to which this filter applies.  Otherwise, this
     // method returns an empty string.

   PtStatus getType(int& type);
     //:Returns the type of this forwarding instruction.
     // The forwarding type indicates whether the forwarding instruction
     // applies unconditionally, upon no answer, or upon busy.

   PtStatus getNoAnswerTimeout(int& time);
     //:Returns the no-answer-timeout value.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

        int mForwardingType;
        int mNoAnswerTimeout;
        int mFilterType;

        UtlString mDestinationUrl;
        UtlString mCallerUrl;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtAddressForwarding_h_
