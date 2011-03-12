//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDCallRouteState_h_
#define _ACDCallRouteState_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/sipXtapi.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ACDCall;

//
// ACDCallRouteState
//
class ACDCallRouteState {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum eRouteState {
      ROUTE_STATE_UNDEFINED,
      IDLE,
      TRYING,
      DISCOVERED,
      CONNECTING,
      ON_HOLD,
      ROUTED,
      FAILED,
      ABORTED,
      TERMINATED
   };

/* ============================ MANIPULATORS ============================== */

   virtual void routeRequestEvent(ACDCall* pAcdCallInstance);
   virtual void acdCallConnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdCallDisconnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdAgentConnectedActiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void acdAgentConnectedInactiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void acdCallTransferModeFailure(ACDCall* pAcdCallInstance);
   virtual void routeRequestTimeoutEvent(ACDCall* pAcdCallInstance);
   virtual void routeRequestAbortEvent(ACDCall* pAcdCallInstance);
   virtual void acdCTransferConnectedEvent(ACDCall* pAcdCallInstance);

/* ============================ ACCESSORS ================================= */

   eRouteState getState(void) { return mState;}
   const char *getStateString(void) { return mStateString;}

   // Factory Method
   static ACDCallRouteState* Instance(eRouteState state);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* ============================ CREATORS ================================== */

   // Constructor
   ACDCallRouteState(eRouteState state) {
      mState = state;
      switch (mState)
      {
         case IDLE: mStateString = "IDLE"; break ;
         case TRYING: mStateString = "TRYING"; break ;
         case DISCOVERED: mStateString = "DISCOVERED"; break ;
         case CONNECTING: mStateString = "CONNECTING"; break ;
         case ON_HOLD: mStateString = "ON_HOLD"; break ;
         case ROUTED: mStateString = "ROUTED"; break ;
         case FAILED: mStateString = "FAILED"; break ;
         case ABORTED: mStateString = "ABORTED"; break ;
         case TERMINATED: mStateString = "TERMINATED"; break ;
         default:
            mStateString = "ROUTE_STATE_UNDEFINED" ;
      }
   }

   // Destructor
   virtual ~ACDCallRouteState() {}

/* ============================ ACCESSORS ================================= */
   virtual OsStatus join(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void dropAgents(ACDCall *pAcdCallInstance);
   virtual void cleanup(ACDCall *pAcdCallInstance);
   virtual void terminate(ACDCall *pAcdCallInstance);
   virtual void failed(ACDCall *pAcdCallInstance);
   virtual void abort(ACDCall *pAcdCallInstance);

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   eRouteState mState;
   const char *mStateString ;
};



//
// ACDCallRouteState_IDLE
//
class ACDCallRouteState_IDLE : public ACDCallRouteState {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Singleton Constructor
   static ACDCallRouteState* Instance(void);

/* ============================ MANIPULATORS ============================== */

   virtual void routeRequestEvent(ACDCall* pAcdCallInstance);
   virtual void acdCallConnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdCallDisconnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void acdCallTransferModeFailure(ACDCall* pAcdCallInstance);
   virtual void routeRequestAbortEvent(ACDCall* pAcdCallInstance);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDCallRouteState_IDLE() : ACDCallRouteState(IDLE) {}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static ACDCallRouteState* mInstance;
};



//
//
// ACDCallRouteState_TRYING
//
class ACDCallRouteState_TRYING : public ACDCallRouteState {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Singleton Constructor
   static ACDCallRouteState* Instance(void);

/* ============================ MANIPULATORS ============================== */

   virtual void acdCallConnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdCallDisconnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdAgentConnectedActiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void routeRequestTimeoutEvent(ACDCall* pAcdCallInstance);
   virtual void routeRequestAbortEvent(ACDCall* pAcdCallInstance);

   void disconnectCallPickUp(ACDCall* pAcdCallInstance);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDCallRouteState_TRYING() : ACDCallRouteState(TRYING) {}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static ACDCallRouteState* mInstance;
   void   failedToRouteCall(ACDCall* pAcdCallInstance, bool bRingNoAnswer);
};


//
// ACDCallRouteState_DISCOVERED
//
class ACDCallRouteState_DISCOVERED : public ACDCallRouteState {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Singleton Constructor
   static ACDCallRouteState* Instance(void);

/* ============================ MANIPULATORS ============================== */

   virtual void acdCallConnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdCallDisconnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdAgentConnectedInactiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void routeRequestAbortEvent(ACDCall* pAcdCallInstance);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDCallRouteState_DISCOVERED() : ACDCallRouteState(DISCOVERED) {}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static ACDCallRouteState* mInstance;
};


//
// ACDCallRouteState_CONNECTING
//
class ACDCallRouteState_CONNECTING : public ACDCallRouteState {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Singleton Constructor
   static ACDCallRouteState* Instance(void);

/* ============================ MANIPULATORS ============================== */

   virtual void acdCallConnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdCallDisconnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void routeRequestAbortEvent(ACDCall* pAcdCallInstance);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDCallRouteState_CONNECTING() : ACDCallRouteState(CONNECTING) {}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static ACDCallRouteState* mInstance;
};


//
// ACDCallRouteState_ON_HOLD
//
class ACDCallRouteState_ON_HOLD : public ACDCallRouteState {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Singleton Constructor
   static ACDCallRouteState* Instance(void);

/* ============================ MANIPULATORS ============================== */

   virtual void acdCallDisconnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdAgentConnectedActiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void routeRequestAbortEvent(ACDCall* pAcdCallInstance);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDCallRouteState_ON_HOLD() : ACDCallRouteState(ON_HOLD) {}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static ACDCallRouteState* mInstance;
};


//
// ACDCallRouteState_ROUTED
//
class ACDCallRouteState_ROUTED : public ACDCallRouteState {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Singleton Constructor
   static ACDCallRouteState* Instance(void);

/* ============================ MANIPULATORS ============================== */

   virtual void acdCallConnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdCallDisconnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdAgentConnectedActiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void acdCallTransferModeFailure(ACDCall* pAcdCallInstance);
   virtual void routeRequestAbortEvent(ACDCall* pAcdCallInstance);
   virtual void acdCTransferConnectedEvent(ACDCall* pAcdCallInstance);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDCallRouteState_ROUTED() : ACDCallRouteState(ROUTED) {}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static ACDCallRouteState* mInstance;
};


//
// ACDCallRouteState_FAILED
//
class ACDCallRouteState_FAILED : public ACDCallRouteState {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Singleton Constructor
   static ACDCallRouteState* Instance(void);

/* ============================ MANIPULATORS ============================== */

   virtual void acdCallDisconnectedEvent(ACDCall* pAcdCallInstance);
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDCallRouteState_FAILED() : ACDCallRouteState(FAILED) {}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static ACDCallRouteState* mInstance;
};


//
// ACDCallRouteState_ABORTED
//
class ACDCallRouteState_ABORTED : public ACDCallRouteState {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Singleton Constructor
   static ACDCallRouteState* Instance(void);

/* ============================ MANIPULATORS ============================== */
   virtual void acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);
   virtual void acdCallDisconnectedEvent(ACDCall* pAcdCallInstance);

/* ============================ MANIPULATORS ============================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDCallRouteState_ABORTED() : ACDCallRouteState(ABORTED) {}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static ACDCallRouteState* mInstance;
};


//
// ACDCallRouteState_TERMINATED (TERMINAL STATE)
//
class ACDCallRouteState_TERMINATED : public ACDCallRouteState {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Singleton Constructor
   static ACDCallRouteState* Instance(void);

/* ============================ MANIPULATORS ============================== */
   virtual void acdCallDisconnectedEvent(ACDCall* pAcdCallInstance);
   virtual void acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDCallRouteState_TERMINATED() : ACDCallRouteState(TERMINATED) {}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static ACDCallRouteState* mInstance;
};

#endif  // _ACDCallRouteState_h_
