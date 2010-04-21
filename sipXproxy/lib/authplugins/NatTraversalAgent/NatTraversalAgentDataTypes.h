//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _NATTRAVERSALAGENTDATATYPES_H_
#define _NATTRAVERSALAGENTDATATYPES_H_

// SYSTEM INCLUDES
#include "utl/UtlContainable.h"
#include "utl/UtlString.h"
#include "utl/UtlInt.h"

// APPLICATION INCLUDES
#include "AuthPlugin.h"

// DEFINES
#define INVALID_MEDIA_RELAY_HANDLE (-1)
#define UNKNOWN_IP_ADDRESS_STRING ("unknown")
#define UNKNOWN_TRANSPORT_STRING  ("unknown")
#define UNKNOWN_PORT_NUMBER       (0)

// CONSTANTS
// TYPEDEFS
typedef UtlInt tMediaRelayHandle;

// FORWARD DECLARATIONS
class MediaBridgePair;
class RegistrationDB;
struct PacketProcessingStatistics;

typedef enum
{
   DIR_CALLEE_TO_CALLER,
   DIR_CALLER_TO_CALLEE
} TransactionDirectionality;

typedef enum
{
   SEND_RECV,
   SEND_ONLY,
   RECV_ONLY,
   INACTIVE,
   NOT_A_DIRECTION
} MediaDirectionality;

typedef enum
{
   PUBLIC,
   REMOTE_NATED,
   LOCAL_NATED,
   UNKNOWN
} LocationCode;

typedef enum
{
   CALLER,
   CALLEE
} EndpointRole;

// FORWARD DECLARATIONS
class NatTraversalRules;

class TransportData : public UtlContainable
{
public:
   TransportData();
   TransportData( const Url& url );
   TransportData( const UtlString& ipAddress, uint16_t portNumber = 5060, const UtlString& transportProtocol = "udp" );
   virtual ~TransportData(){}

   //UtlContainable methods
   virtual UtlContainableType getContainableType() const;
   virtual unsigned hash() const;
   virtual int compareTo(UtlContainable const *) const;

   //GETTERS
   const UtlString& getAddress( void ) const;
   int              getPort( void ) const;
   const UtlString& getTransportProtocol( void ) const;

   //SETTERS
   void setAddress( const UtlString& address );
   void setPort( int port );
   void setTransportProtocol( const UtlString& transport );

   //STRINGER
   void toUrlString( UtlString& outputString ) const;

   //OPERATORS
   bool isEqual( const TransportData& rhs ) const;
   bool isInitialized( void ) const;

   //MANIPULATORS
   virtual void fromUrl( const Url& url );

   //MISC
   static const UtlContainableType TYPE;

protected:
   //MISC
   const UtlString& getTransportDataType( void ) const;

   UtlString mTransportType;     ///< "Public", "Private", or "unknown"
   UtlString mAddress;
   int mPort;
   UtlString mTransportProtocol; ///< "transport" parameter value or "unknown"
};

class NativeTransportData : public TransportData
{
public:
   NativeTransportData( const Url& url );
   virtual void fromUrl( const Url& url );
   virtual ~NativeTransportData(){}
};


class PublicTransportData : public TransportData
{
public:
   PublicTransportData( const Url& url );
   virtual void fromUrl( const Url& url );
   virtual ~PublicTransportData(){}
};

/// The EndpointDescriptor class is used to analyze a URI to look for
/// proprietary location markers ("x-sipX-nonat" and "x-sipX-privcontact")
/// and derive the native and public IP Address:port;transport
/// information relative to a given SIP endpoint.  Furthermore, it is
/// responsible for computing the location of that endpoint relative to
/// the sipXecs based on its public and native transport information and
/// the content of the NatTraversalRules.  The EndpointDescriptor can classify
/// the location of an enpoint into one of 4 categories:
/// PUBLIC:       The endpoint is not located behind any NAT.
/// LOCAL_NATED:  The endpoint is located in the same local private entwork
///               as the sipXecs and both are behind the same NAT.
/// REMOTE_NATED: The endpoint is behind a remote NAT
/// UNKNOWN:      The location of the endpoint cannot be determined.
///
/// Note that in some call scenarios (call pick-up and call park, notably)
/// the URIs that are provided do not contain our proprietary location markers.
/// When a pointer to a RegistrationDB is provided, the EndpointDescriptor
/// will search the Registration DB looking for a Contact entry matching
/// the supplied URI's user@hostport hoping to find the location markers that
/// are needed to establish the location of the endpoint.

class EndpointDescriptor
{
public:
   EndpointDescriptor( const Url& url, const NatTraversalRules& natRules, const RegistrationDB* pRegistrationDB = NULL );

   // GETTERS
   const TransportData& getNativeTransportAddress( void ) const;
   const TransportData& getPublicTransportAddress( void ) const;
   LocationCode getLocationCode( void ) const;

   void toString( UtlString& outputString ) const;

protected:
private:
   NativeTransportData  mNativeTransport;
   PublicTransportData  mPublicTransport;
   LocationCode         mLocation;
   Url                  mCurrentContact;

   LocationCode computeLocation( const Url& url,
                                 const NatTraversalRules& natRules,
                                 const RegistrationDB* pRegistrationDB );
   LocationCode computeLocationFromPublicAndNativeTransports( const NatTraversalRules& natRules );
   LocationCode computeLocationFromRegDbData( const Url& url,
                                              const NatTraversalRules& natRules,
                                              const RegistrationDB* pRegistrationDB );
   LocationCode computeLocationFromNetworkTopology( const NatTraversalRules& natRules );
};

/// This class is an abstraction of a SIP endpoint involved in a media session.
/// It is used to store that endpoint's media address, RTP and RTCP ports.
class MediaEndpoint
{
public:
   MediaEndpoint( const MediaEndpoint& referenceMediaEndpoint );
   MediaEndpoint( const SdpBody& sdpBody, size_t mediaDescriptionIndex );
   MediaEndpoint();

   /// Returns true if chnages got made to the encapsulated data.  Returns false if encapsulated data already has values contained in SDP.
   bool              setData    ( const SdpBody& sdpBody, size_t mediaDescriptionIndex );
   const UtlString&  getAddress ( void ) const;
   int               getRtpPort ( void ) const;
   int               getRtcpPort( void ) const;
   MediaEndpoint&    operator=( const MediaEndpoint& rhs );

private:
   UtlString mAddress;
   int       mRtpPort;
   int       mRtcpPort;
};

/// The MediaDescriptor class is used to store the information related
/// a given media negotiation for both the caller (original UAC) and
/// the callee (original UAS) as well as some information common to both
/// the caller and the callee (session directionality and type for example).
/// The information stored in this class is mainly extracted the offer's and
/// answer's Media Description section of an SDP Offer/Answer.  Each instance
/// of a MediaDescriptor is responsible for storing the information of a
/// single Media Description section inside an SDP that is why all the
/// APIs that accept an SDP body as a parameter also accept an index parameter
/// which represents the 0-based index of the Media Description section
/// that pay attention to.
///
/// Because this class stores information about both the caller and callee
/// endpoints, method that are used to save information about a particular endpoint
/// also accept a parameter that describes the role (see EndpointRole) of the
/// endpoint to which the information pertains.  Note that the designation
/// of the caller and callee is established at call set-up time and
/// does not change thoughout the lifetime of the dialog.  That means
/// that the CALLER endpoint role will be attributed to the original
/// UAC of the dialog and vice versa.
///
/// On top of collecting information from the offer/answer SDPs, this class
/// play the important role of remembering all the tentative media relay handles
/// and currently used media relay handle.  These handles link back to a symmitron
/// bridge that actually performs the media relay function.
class MediaDescriptor
{
public:
   MediaDescriptor( const SdpBody& sdpBody, size_t index, EndpointRole endpointRole );
   MediaDescriptor( const MediaDescriptor& referenceMediaDescriptor );
   const UtlString&     getType    ( void ) const;
   MediaDirectionality  getDirectionality( void ) const;
   void                 setDirectionalityOverride( MediaDirectionality override );
   MediaDirectionality  getDirectionalityOverride( void ) const;
   void                 setMediaTypeAndDirectionalityData( const SdpBody& sdpBody, size_t index );
   const MediaEndpoint& getEndpoint( EndpointRole endpointRole ) const;
   /// Returns true if 'set' operation resulted in a change in the saved data. Returns false if saved already matched new values
   bool                 setEndpointData( const SdpBody& sdpBody, size_t index, EndpointRole endpointRole );
   void                 setCurrentMediaRelayHandle( const tMediaRelayHandle handle );
   tMediaRelayHandle    getCurrentMediaRelayHandle( void ) const;
   void                 clearCurrentMediaRelayHandle( void );
   void                 setTentativeInitialMediaRelayHandle( const tMediaRelayHandle handle );
   tMediaRelayHandle    getTentativeInitialMediaRelayHandle( void ) const;
   void                 clearTentativeInitialMediaRelayHandle( void );
   void                 setTentativeNonInitialMediaRelayHandle( const tMediaRelayHandle handle );
   tMediaRelayHandle    getTentativeNonInitialMediaRelayHandle( void )  const;
   void                 clearTentativeNonInitialMediaRelayHandle( void );
   static MediaDirectionality sdpDirectionalityAttributeToMediaDirectionalityValue( const SdpBody& sdpBody, size_t index );
   static void                mediaDirectionalityValueToSdpDirectionalityAttribute( const MediaDirectionality valueToConvert, UtlString& conversion );
   MediaDescriptor&     operator=( const MediaDescriptor& rhs );

private:
   // Media data variables
   UtlString           mType;                   // audio, video or other
   size_t              mMediaDescriptionIndex;  // position of the media desciption section related to this object in the SDP body
   MediaDirectionality mDirectionality;         // directionality as seen in the SDP offer
   MediaDirectionality mDirectionalityOverride; // directionality override as imposed by the NAT traversal logic
   MediaEndpoint       mCaller;
   MediaEndpoint       mCallee;

   // Media Relay handle variables
   tMediaRelayHandle mCurrentMediaRelayHandle;
   tMediaRelayHandle mTentativeInitialMediaRelayHandle;
   tMediaRelayHandle mTentativeNonInitialMediaRelayHandle;

   const MediaEndpoint& getCallerEndpoint( void ) const;
   bool                 setCallerEndpointData( const SdpBody& sdpBody, size_t index );
   const MediaEndpoint& getCalleeEndpoint( void ) const;
   bool                 setCalleeEndpointData( const SdpBody& sdpBody, size_t index );
};

/// Structure used to save packet processing information.
struct PacketProcessingStatistics
{
   PacketProcessingStatistics();
   intptr_t      mNumberOfPacketsProcessed;
   unsigned long mEpochTimeOfLastPacketsProcessed;
};

/// This class is an abstration of a single session capable of relaying one RTP + one RTCP
/// streams between two endpoints.
class MediaRelaySession : public UtlContainable
{
public:
   MediaRelaySession( const tMediaRelayHandle& uniqueHandle,
                      int callerPort,
                      int calleePort,
                      MediaBridgePair *pAssociatedMediaBridgePair,
                      bool isaCloneOfAnotherMediaRelaySession = false );
   ///< c'tor used when creating a media relay session that is a clone of an existing one.

   //UtlContainable methods
   virtual UtlContainableType getContainableType() const;
   virtual unsigned hash() const;
   virtual int compareTo(UtlContainable const *) const;

   // GETTERS
   const tMediaRelayHandle& getUniqueHandle( void ) const { return mUniqueHandle; }
   bool areCallerAndCalleeRtpPortsSwapped( void ) const { return mbCallerAndCalleeRtpPortsSwapped; }
   int getRtpRelayPort( EndpointRole endpointRole ) const;
   bool isaCloneOfAnotherMediaRelaySession( void ) const;
   MediaBridgePair* getAssociatedMediaBridgePair( void ) const { return mpAssociatedMediaBridgePair; }
   const PacketProcessingStatistics& getPacketProcessingStats( void ){ return mPacketProcessingStats; }

   // SETTERS
   void setPacketProcessingStats( const PacketProcessingStatistics& newStats );

   // LINK COUNT MANIPULATIONS
   ssize_t getLinkCount( void ) const;
   ssize_t incrementLinkCount( void );
   ssize_t decrementLinkCount( void );

   // Misc
   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

private:
   tMediaRelayHandle mUniqueHandle;    // Handle that uniquely identifies this instance of the class
   int               mCallerRtpPort;
   int               mCalleeRtpPort;
   bool              mbIsaCloneOfAnotherMediaRelaySession;
   bool              mbCallerAndCalleeRtpPortsSwapped;  // When session is cloned from another, it indicates
                                                        // whether this session has its caller and caller RTP
                                                        // ports reversed compared to the original
   MediaBridgePair*  mpAssociatedMediaBridgePair;
   ssize_t           mLinkCount;
   PacketProcessingStatistics mPacketProcessingStats;
};

#endif // _NATTRAVERSALAGENTDATATYPES_H_
