//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _DIALOGTRACKERSTATES_H_
#define _DIALOGTRACKERSTATES_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlFsm.h"
#include "NatTraversalAgentDataTypes.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class DialogTracker;
class SipMessage;

class DialogTrackerState
{
public:
   /***
    * Returns the parent state of this state. As required by StateAlg (in UtlFsm.h).
    */
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;

   /**
    * Returns the initial state contained in this state. As required by StateAlg
    */
   virtual const DialogTrackerState* GetInitialState( DialogTracker& impl ) const;

   /**
    * Called whenever this state is entered. As required by StateAlg
    */
   virtual void DoEntryAction( DialogTracker& impl ) const;

   /**
    * Called whenever this state is exited. As required by StateAlg
    */
   virtual void DoExitAction( DialogTracker& impl ) const;

   virtual const char* name( void ) const;

   virtual ~DialogTrackerState(){};

   // State machine events
   // See NAT_Traversal_Design_Doc.doc section 4.2.2 for definitions.
   virtual bool InviteRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
   virtual bool AckRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
   virtual bool ByeRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
   virtual bool PrackRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
   virtual bool UpdateRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
   virtual void ProvisionalResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void RedirectionResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void FailureResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void CleanUpTimerTick( DialogTracker& impl ) const;
protected:
   void ChangeState( DialogTracker& impl,
                      const DialogTrackerState* targetState ) const;

};

// State machine states.
// See NAT_Traversal_Design_Doc.doc section 4.2.2 for definitions.

class WaitingForInvite : public DialogTrackerState
{
public:
   virtual const char* name( void ) const;
   virtual ~WaitingForInvite(){};
   virtual void DoEntryAction( DialogTracker& impl ) const;

   // State machine events relevant for this state
   virtual bool InviteRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
   virtual void CleanUpTimerTick( DialogTracker& impl ) const;
};

class TimeBoundState: public DialogTrackerState
{
public:
   virtual const char* name( void ) const;
   virtual ~TimeBoundState(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( DialogTracker& impl ) const;
   virtual void CleanUpTimerTick( DialogTracker& impl ) const;
};

class Negotiating : public TimeBoundState
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~Negotiating(){};

   // State machine events relevant for this state
   virtual void FailureResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
};


class WaitingForAckForInvite : public TimeBoundState
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~WaitingForAckForInvite(){};

   // State machine events relevant for this state
   virtual bool AckRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
};

// The ProcessingPrack state is a super-state that serves as a parent to all the
// states that are used to handle the PRACK-related events that can happen during
// 'fast start' media negotitation.  This super-state is used to help process
// the provisional and 200 OK responses to INVITEs that can be received while waiting
// for the PRACK responses.  The state applies the necessary NAT compensation
// transformations on the SDP bodies they may carry. This state was created in
// response to XX-5665 which showed that 200 OK for INVITE can be received
// before or after 200 OK for PRACK.  As well, other SDP-carying provisional responses can
// be received while waiting for PRACK responses.  This state will catch all responses
// that carry SDPs and manipulate them for NAT traversal irrespective of the order
// that they are received in.
//
// Prior to the introduction of this state, the FSM imposed that 200 OK for PRACK be
// received *before* 200 OK for INVITE and did not handle extra provisional responses
// carrying SDPs while waiting for the PRACK responses.
class ProcessingPrack : public Negotiating
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~ProcessingPrack(){};

   // State machine events relevant for this state
   virtual void ProvisionalResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
};

class Moribund : public DialogTrackerState
{
public:
   virtual const char* name( void ) const;
   virtual void DoEntryAction( DialogTracker& impl ) const;
   virtual ~Moribund(){};
};

class WaitingForMediaOffer : public Negotiating
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~WaitingForMediaOffer(){};

   // State machine events relevant for this state
   virtual void ProvisionalResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
};

class WaitingFor200OkWithMediaOffer : public Negotiating
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~WaitingFor200OkWithMediaOffer(){};

   // State machine events relevant for this state
   virtual void ProvisionalResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
};

class WaitingForMediaAnswer : public Negotiating
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~WaitingForMediaAnswer(){};

   // State machine events relevant for this state
   virtual void ProvisionalResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
};

class WaitingForPrack : public ProcessingPrack
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~WaitingForPrack(){};

   // State machine events relevant for this state
   virtual bool PrackRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
};

class WaitingForAckWithAnswerForInvite : public Negotiating
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~WaitingForAckWithAnswerForInvite(){};

   // State machine events relevant for this state
   virtual bool AckRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
};

class WaitingForPrackWithMediaAnswer : public Negotiating
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~WaitingForPrackWithMediaAnswer(){};

   // State machine events relevant for this state
   virtual bool PrackRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
};


class WaitingFor200OkForSlowStartPrack : public Negotiating
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~WaitingFor200OkForSlowStartPrack(){};

   // State machine events relevant for this state
   virtual void SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void FailureResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
};

class WaitingFor200OkForPrack : public ProcessingPrack
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~WaitingFor200OkForPrack(){};

   // State machine events relevant for this state
   virtual void SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void FailureResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
};

class WaitingFor200OkWithAnswerForPrack : public ProcessingPrack
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~WaitingFor200OkWithAnswerForPrack(){};

   // State machine events relevant for this state
   virtual void SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
   virtual void FailureResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const;
};

class ProcessingPrackWaitingForAckforInvite : public ProcessingPrack
{
public:
   virtual const char* name( void ) const;
   virtual const DialogTrackerState* GetParent( DialogTracker& impl ) const;
   virtual ~ProcessingPrackWaitingForAckforInvite(){};

   // State machine events relevant for this state
   virtual bool AckRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality dir, const char* address, int port ) const;
};

#endif // _DIALOGTRACKERSTATES_H_
