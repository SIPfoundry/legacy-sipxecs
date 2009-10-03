//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _OrbitListener_h_
#define _OrbitListener_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <tao/TaoAdaptor.h>
#include <tao/TaoDefs.h>
#include <ParkedCallObject.h>
#include <utl/UtlHashMap.h>
#include <filereader/OrbitFileReader.h>

// DEFINES
#define ORBIT_CONFIG_FILENAME     "orbits.xml"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CallManager;
class TaoString;
class OrbitData;

//: Dummy DTMF listener.
class DummyListener : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   DummyListener();
   //:Default constructor

   DummyListener(const DummyListener& rDummyListener);
   //:Copy constructor

   virtual ~DummyListener();
   //:Destructor

/* ============================ MANIPULATORS ============================== */

   DummyListener& operator=(const DummyListener& rhs);
   //:Assignment operator

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

};


//: Object that listens for incoming calls for the Park Server.
//  This object handles an arbitrary set of orbits, described by one orbits.xml
//  file.
class OrbitListener : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OrbitListener(CallManager* callManager, ///< Call Manager for signaling
                 int lifetime,             ///< Max. parked call lifetime in secs.
                 int blindXferWait,        ///< Max. time for blind xfer. in secs.
                 int keepAliveTime,        ///< Periodic time for sending keepalives in secs.
                 int consXferWait          ///< Max. time for cons. xfer. in secs.
      );
   //:Default constructor

   ~OrbitListener();

/* ============================ MANIPULATORS ============================== */

   UtlBoolean handleMessage(OsMsg& rMsg);
     //: Method to process messages which get queued on this
     /**  The is the method that does all the work.  Telephony
       *  events get queued on this which consequently get
       *  processed in handleMessage in the contect of this
       *  task.
       */

/* ============================ ACCESSORS ================================= */
void dumpCallsAndTransfers();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    void dumpTaoMessageArgs(TaoObjHandle eventId, TaoString& args);

    OsStatus validateOrbit(const UtlString& callId,
                           const UtlString& address,
                           UtlString& orbit,
                           UtlString& audio,
                           int& timeout,
                           int& keycode,
                           int& capacity);
    bool isCallRetrievalInvite(const char* callId, const char* address);

    // Set up the data structures for a new call that isn't a call-retrieval
    // call.  Triggered by CONNECTION_OFFERED.
    void setUpParkedCallOffered(const UtlString& callId,
                                const UtlString& address,
                                const UtlString& orbit,
                                const UtlString& audio,
                                int timeout,
                                int keycode,
                                int capacity,
                                const TaoString& arg);

    // Set up the call state for a new call that isn't a call-retrieval
    // call.  Triggered by CONNECTION_ESTABLISHED.
    void setUpParkedCallEstablished(const UtlString& callId,
                                    const UtlString& address,
                                    const TaoString& arg);

    // Do the work for a call-retrieval call.
    void setUpRetrievalCall(const UtlString& callId,
                            const UtlString& address);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    OrbitListener(const OrbitListener& rOrbitListener);
     //:Copy constructor

    OrbitListener& operator=(const OrbitListener& rOrbitListener);
     //:Assignment operator

    // The Call Manager to use for signaling.
    CallManager* mpCallManager;

    // The maximum lifetime to allow for parked calls.
    OsTime mLifetime;

    // The time to allow for a blind transfer.
    OsTime mBlindXferWait;

    // The periodic time to send keepalive signals.
    OsTime mKeepAliveTime;

    // The time to allow for a consultative transfer.
    OsTime mConsXferWait;

    ParkedCallObject* getOldestCallInOrbit(const UtlString& orbit,
                                           UtlString& callId,
                                           UtlString& address);

    // Get the number of calls in an orbit.
    // If onlyAvailable, then do not count calls that have a transfer in progress.
    int getNumCallsInOrbit(const UtlString& orbit,
                           UtlBoolean onlyAvailable);

    // Find the ParkedCallObject with a given mSeqNo, or return NULL.
    ParkedCallObject* findBySeqNo(int seqNo);

    // Maps original call-Ids of parked calls (and the calls picking them up)
    // to their ParkedCallObjects.
    UtlHashMap mCalls;

    // Object to manage reading and updating orbit file information.
    OrbitFileReader mOrbitFileReader;

    // Dummy DTMF listener.
    DummyListener mListener;

    // Map from the call-IDs of transfer pseudo-calls to the call-IDs of the
    // real calls they are reporting on.
    UtlHashMap mTransferCalls;

/* //////////////////////////// PRIVATE /////////////////////////////////// */

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OrbitListener_h_
