//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _MEDIARELAY_H_
#define _MEDIARELAY_H_

// SYSTEM INCLUDES
#include <vector>
#include "utl/UtlString.h"
#include "net/Url.h"
#include "os/OsMutex.h"
#include "os/OsServerTask.h"
#include "os/OsMsg.h"
#include "os/OsTimer.h"
#include "os/OsNotification.h"

// APPLICATION INCLUDES
#include "AuthPlugin.h"
#include "NatTraversalAgentDataTypes.h"

// DEFINES
#define INVALID_MEDIA_RELAY_HANDLE (-1)
// CONSTANTS
// TYPEDEFS
typedef UtlInt tMediaRelayHandle;
// FORWARD DECLARATIONS
class MediaRelaySession;
class XmlRpcRequest;
class XmlRpcResponse;
class MediaRelay;

/**
 * This class encapsulates all the information relative
 * to a Sym obtained from the Symmitron.
 * Please refer to the Symmitron documentation for more
 * information about Syms.
 */
class Sym : public UtlContainable
{
   public:
      Sym( UtlString& id, UtlString& localAddress, int port );
      UtlString getId( void ) const;
      UtlString getLocalAddress( void ) const;
      int       getPort( void ) const;

      //UtlContainable methods
      virtual UtlContainableType getContainableType() const;
      virtual unsigned hash() const;
      virtual int compareTo(UtlContainable const *) const;
      static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   private:
      UtlString mId;
      UtlString mLocalAddress;
      int mPort;
};

/**
 * This class encapsulates all the information relative
 * to a Bridge obtained from the Symmitron.
 * Please refer to the Symmitron documentation for more
 * information about Bridges.
 */
class Bridge : public UtlContainable
{
   public:
      Bridge( UtlString& id, Sym* pEndpoint1Sym, Sym* pEndpoint2Sym );
      UtlString getId( void ) const;
      const Sym* getEndpoint1Sym( void ) const;
      const Sym* getEndpoint2Sym( void ) const;

      //UtlContainable methods
      virtual UtlContainableType getContainableType() const;
      virtual unsigned hash() const;
      virtual int compareTo(UtlContainable const *) const;
      static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   private:
      UtlString mId;
      Sym* mpEndpoint1Sym;
      Sym* mpEndpoint2Sym;
};

/**
 * A Symmitron Bridge effectively relays a UDP data stream
 * between two endpoints.  The NAT Traversal feature uses
 * such bridges to relay audio media streams between endpoints
 * that could otherwise not stream to eachother due to the
 * presence of NATs interference.  An media stream sent
 * using RTP consists in two streams; an RTP stream that
 * carries the voice and an RTCP stream that carries quality
 * metrics.  The MediaBridgePair is a class that encapsulates
 * a pair of bridges, one being use to relay RTP and the other
 * used to relay RTCP.
 */
class MediaBridgePair : public UtlContainable
{
   public:
      MediaBridgePair( Bridge* pRtpBridge, Bridge* pRtcpBridge );
      const Bridge* getRtpBridge( void ) const;
      const Bridge* getRtcpBridge( void ) const;

      //UtlContainable methods
      virtual UtlContainableType getContainableType() const;
      virtual unsigned hash() const;
      virtual int compareTo(UtlContainable const *) const;
      static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   private:
      Bridge* mpRtpBridge;
      Bridge* mpRtcpBridge;
};

/** Class used to communicate with the Media Relay (a.k.a Symmitron) in an
 * asynchronous fashion.  The services of this class are required when XML-RPC
 * requests to the media relay need to be sent on the fast path since
 * sending XML-RPC requests is a blocking operation.
 */
class AsynchMediaRelayRequestSender : public OsServerTask
{
public:
   AsynchMediaRelayRequestSender( MediaRelay* pOwningMediaRelay );
   virtual ~AsynchMediaRelayRequestSender();

   virtual UtlBoolean handleMessage(OsMsg& rMsg);
   void setSymmitronInstanceHandle( const UtlString& symmitronHandle );

   //Asynch requests to Symmitron
   void pauseBridge( const UtlString& controllerHandle, const UtlString& bridgeId );
   void resumeBridge( const UtlString& controllerHandle, const UtlString& bridgeId );
   void setDestination( const UtlString& controllerHandle, const UtlString& symId, const UtlString& ipAddress, int port, int keepAliveTime = 0 );
   void pauseSym( const UtlString& controllerHandle, const UtlString& symId );
   void resumeSym( const UtlString& controllerHandle, const UtlString& symId );
   void setSymTimeout( const UtlString& controllerHandle, const UtlString& symId, int timeout );
   void ping( const UtlString& controllerHandle );
   void queryBridgeStatistics( const UtlString& controllerHandle, const UtlString& bridgeId, void* opaqueData );
   OsStatus postMessageIfStarted( const OsMsg& rMsg,
                                  const OsTime& rTimeout=OsTime::OS_INFINITY,
                                  UtlBoolean sentFromISR=FALSE );

private:
   MediaRelay* mpOwningMediaRelay;
   UtlString   mReferenceSymmitronInstanceHandle;
};

/**
 * Message object used to communicate to AsynchMediaRelayRequestSender
 * via its message queue.
 */
class AsynchMediaRelayMsg : public OsMsg
{
public:

   enum EventSubType
   {
      SYMMITRON_PAUSE_BRIDGE    = 1,
      SYMMITRON_RESUME_BRIDGE   = 2,
      SYMMITRON_SET_DESTINATION = 3,
      SYMMITRON_PAUSE_SYM       = 4,
      SYMMITRON_RESUME_SYM      = 5,
      SYMMITRON_SET_SYM_TIMEOUT = 6,
      SYMMITRON_PING            = 7,
      SYMMITRON_GET_BRIDGE_STATS= 8
   };

   //TODO: all these constructors that are specialized for EventSubType is
   //      really bad OO.  Replace with proper message ebstractions.
   // constructor to use for SYMMITRON_PAUSE_BRIDGE, SYMMITRON_RESUME_BRIDGE,
   // SYMMITRON_PAUSE_SYM and SYMMITRON_RESUME_SYM
   AsynchMediaRelayMsg( EventSubType eventSubType,
                        const UtlString& controllerHandle,
                        const UtlString& subId /*symId or bridgeId depending on eventSubType */ );

   // constructor to use for SYMMITRON_SET_DESTINATION
   AsynchMediaRelayMsg( const UtlString& controllerHandle,
                        const UtlString& symId,
                        const UtlString& ipAddress,
                        int port,
                        int keepAliveTime );

   // constructor to use for SYMMITRON_SET_SYM_TIMEOUT
   AsynchMediaRelayMsg( const UtlString& controllerHandle,
                        const UtlString& symId,
                        int timeout );

   // constructor to use for SYMMITRON_GET_BRIDGE_STATS
   AsynchMediaRelayMsg( const UtlString& controllerHandle,
                        const UtlString& bridgeId,
                        void* opaqueData );

   // constructor to use for SYMMITRON_PING
   AsynchMediaRelayMsg( const UtlString& controllerHandle );

   AsynchMediaRelayMsg(const AsynchMediaRelayMsg& rOsMsg);
     //:Copy constructor

   virtual ~AsynchMediaRelayMsg(){};
     //:Destructor

   virtual OsMsg* createCopy(void) const;

   // Component accessors.
   const UtlString& getControllerHandle( void ) const;
   const UtlString& getSymId( void ) const;
   const UtlString& getBridgeId( void ) const;
   const UtlString& getIpAddress( void ) const;
   int getPort( void ) const;
   int getTimeout( void ) const;
   int getKeepAliveTime( void ) const;
   void* getOpaqueData( void ) const;

protected:
   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

private:
   UtlString mControllerHandle;
   UtlString mSubId; // if eventSubType is SYMMITRON_PAUSE_SYM or SYMMITRON_RESUME_SYM then
                     // mSubId contains a Sym ID, otherwise it contains a Bridge Id
   UtlString mIpAddress;
   int mPort;
   int mTimeout;
   int mKeepAliveTime;
   void* mpOpaqueData;
};

/**
 * The class is used to abstract the Symmitron which is the external process
 * that actually performs the media relaying.  Applicatiosn needing to use the
 * services of the Symmitron need to isntantiate and initialize() a MediaRelay
 * object which will take care of all the communication with the Symmitron.  The
 * API it exposes lets the application perform all the necessary operations
 * to utilize the services of the Symmitron.
 */
class MediaRelay : public OsNotification
{
   public:
     MediaRelay();

     ~MediaRelay();

     /**
      * Method used to notify the MediaRelay class that a Symmitron reset
      * was detected.
      */
     void notifySymmitronResetDetected( const UtlString& newSymmitronInstanceHandle );

     /**
      * Method used to notify the MediaRelay class that the bridge stats
      * it has queried have been received
      */
     void notifyBridgeStatistics( const UtlString& bridgeId, intptr_t numberOfPacketsProcessed, void* opaqueData );

     /**
      * Method to call to initialize the MediaRelay object.
      *  - publicAddress: [input] public IP address of Symmitron.
      *                   If the symmitron is behind a NAT, that value should
      *                   be the public IP address of that NAT otherwise it is
      *                   the IP address of the machine it is running on.
      *  - nativeAddress: [input] IP address of the machine the Symmitron is running on
      *  - bXmlRpcSecured: [input] true will send XMLRPC over HTTPS, otherwise it will use HTTP.
      *  - isPartOfsipXLocalPrivateNetwork: [input] True of the symmitron is part
      *                                     of the same local private network as the
      *                                     application that will utilize the media relay.
      *  - xmlRpcPort: [input] Port number to utilize to communicate with Symmitron.
      *  - maxMediaRelaySessions: [input] maximum of media relay sessions to allocate
      *
      * Returns: true for success and false for failure.
      */
      bool initialize( const  UtlString& publicAddress,
                       const  UtlString& nativeAddress,
                       bool   bXmlRpcSecured,
                       bool   isPartOfsipXLocalPrivateNetwork,
                       int    xmlRpcPort,
                       size_t maxMediaRelaySessions );

      /**
       * Getter for isPartOfsipXLocalPrivateNetwork setting passed to initialize() method
       */
      bool isPartOfsipXLocalPrivateNetwork( void ) const{ return mbIsPartOfsipXLocalPrivateNetwork; }

      /**
       * Getter for publicAddress setting passed to initialize() method
       */
      const UtlString& getPublicAddress( void ) const{ return mPublicAddress; }

      /**
       * Getter for nativeAddress setting passed to initialize() method
       */
      const UtlString& getNativeAddress( void ) const{ return mNativeAddress; }

      /**
       * Method used to obtain a media relay session capable of relaying RTP and
       * RTCP streams.
       *
       * - relayHandle: [output] handle to allocated session
       * - enpoint1RelayRtpPort: [output] RTP port that symmitron will expect traffic from.
       *                         Note that RTCP port is +1;
       * - enpoint2RelayRtpPort: [output] oOther RTP port that symmitron will expect traffic from
       *                         Note that RTCP port is +1;
       */
      bool allocateSession( tMediaRelayHandle& relayHandle, int& enpoint1RelayRtpPort, int& enpoint2RelayRtpPort );
      tMediaRelayHandle cloneSession( const tMediaRelayHandle& relayHandleToClone, bool doSwapCallerAndCallee );
      bool deallocateSession( const tMediaRelayHandle& relayHandle );
      bool setDirectionMode( const tMediaRelayHandle& relayHandle, MediaDirectionality mediaRelayDirectionMode ); // Directionality referenced from the caller
      bool linkSymToEndpoint( const tMediaRelayHandle& relayHandle,
                              const UtlString& endpointIpAddress, // empty string means that IP needs to be autolearned
                              int endpointRtpPort,                // port value of 0 means that port needs to be autolearned
                              int endpointRtcpPort,
                              EndpointRole ownerOfSymToLink );
      ssize_t incrementLinkCountOfMediaRelaySession( const tMediaRelayHandle& handle );
      int getRtpRelayPortForMediaRelaySession( const tMediaRelayHandle& handle, EndpointRole endpointRole );
      void deallocateAllSymmitronResourcesAndSignOut( void );
      const Url& getXmlRpcServerUrl( void ) const;
      static UtlHashMap* executeAndValudateSymmitronRequest( XmlRpcRequest& requestToSend, UtlString& symmitronInstanceHandle, int& errorCode, UtlString& errorDescription, XmlRpcResponse& xmlRpcResponse, bool bRetryFailedConnection = true );
      bool getPacketProcessingStatsForMediaRelaySession( const tMediaRelayHandle& handle, PacketProcessingStatistics& stats );

      // OsNotification virtual method implementation
      virtual OsStatus signal(intptr_t eventData);

   private:
      // misc
      MediaRelaySession* getSessionByHandle( const tMediaRelayHandle& handle );

      // Symmitron operations
      bool preAllocateSymmitronResources( void );
      UtlHashMap* getAndValidateStandardMap( XmlRpcResponse& responseToValidate, UtlString& symmitronInstanceHandle, int& errorCode, UtlString& errorDescription );
      bool createPausedBridgeOnSymmitron( Sym* pEndpoint1Sym, Sym* pEndpoint2Sym, UtlString& returnedBridgeId, UtlString& symmitronInstanceHandle, int& errorCode, UtlString& errorDescription );
      bool addSymToBridge( UtlString& symId, UtlString& bridgeId, UtlString& symmitronInstanceHandle, int& errorCode, UtlString& errorDescription );

      // clean up
      void cleanUpEverything( void );

      UtlString                      mPublicAddress;            // public IP address of the media relay
      UtlString                      mNativeAddress;            // native address of the media relay
      bool                           mbIsPartOfsipXLocalPrivateNetwork;  // indicates whether the media relay is inside the same local private network as sipXecs
      int                            mXmlRpcPort;               // XML-RPC port that Symmitron is listening on
      size_t                         mMaxMediaRelaySessions;    // Configured maximum number of media relay sessions allowed
      UtlHashMap                     mActiveMediaRelaySessions; // maps media relay session handles to MediaRelaySession objects.
      int                            mRelaySessionHandle;       // Next availalble handle to assign to a MediaRelaySesion we create
      Url                            mSymmitronUrl;             // URL to use to reach Symmitron's XML-RPC server
      UtlString                      mOurInstanceHandle;        // Instance handle that we are supplying to Symmitron in all our request
      UtlString                      mSymmitronInstanceHandle;  // last Instance handle value we received from Symmitron
      bool                           mbSignedInWithSymmitron;   // Indicates whwther or not the signIn request has been sent to Symmitron and has been accepted
      UtlSortedList                  mSymList;                  // List of all the syms we have pre-allocated on thr Symmitron
      std::vector<MediaBridgePair*>  mAvailableMediaBridgePairsList;  // Tracks list of available MediaBridgePairs
      std::vector<MediaBridgePair*>  mBusyMediaBridgePairsList;       // Tracks list of the MediaBridgePairs currently being used to relay media
      AsynchMediaRelayRequestSender  mAsynchMediaRelayRequestSender;  // Instance of class used to send requests to the symmitron without blocking the fasts path
      OsMutex                        mMutex;
      OsTimer                        mGenericTimer;                 // Timer used to trigger pings to the symmitron and to query bridge stats
      ssize_t                        mGenericTimerTickCounter;      // Counts the number of generic timer ticks since the beginning
      bool                           mbPollForSymmitronRecovery;    // Flag set to true when the connection to Symmitron is lost and needs to be recovered.
};

#endif
