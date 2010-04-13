//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtGatewayInterface_h_
#define _PtGatewayInterface_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtMediaCapabilities;
class PtAudioCodec;

//: Abstract gateway interface used to obtain call setup information from
//: a media gateway.
// To implement a media gateway using PTAPI one implements this class by
// deriving from it, creating an instance and registering it on the
// PtTerminal which represents the gateway via the
// PtTerminal::setGatewayInterface method.  All of the methods on this
// class must be implemented.
// <BR>
// A call setup with the the gateway (incoming or out going) starts with
// the call to the <I>reserveLine</I> method.  The gateway uses this method to
// indicate that whether it has resources to accept or initiate a call.
// Regardless of whether the gateway has resources for a line, the gateway
// must return a line handle which the gateway may use to correlate
// subsequent calls to methods on this class related to the same
// line/terminalConnection.  If <I>reserveLine</I> returned with an availability
// other than LINE_AVAILABLE, the provider server will call the releaseLine
// method and the other side of the call will get appropriate indication.
// <BR>
// If <I>reserveLine</I> returned with an availability of LINE_AVAILABLE,
// the provider service will call the <I>getLineCapabilities</I>
// method to get the encoding capabilities for both sending and receiving
// of RTP packets.  If a codec is found which is compatible with the other
// end of the call the provider will subsequently call the <I>startLineRtpSend</I>
// and <I>startLineRtpReceive</I> methods.  However the order and timing in which
// these two methods are called will vary depending upon which side
// initiated the call and the implementation of the phone at the other end.
// <BR>
// When either side disconnects the call, the provider server will then
// invoke the stopLineRtpSend and stopLineRtpReceive methods.  Again the
// order in which these are invoked may vary.  Finally the releaseLine
// method will be invoked to indicate that the line is no longer needed.
// <BR>
// <H3>Other PTAPI Interfaces Relevent to Gateways</H3>
// For incoming calls (from other phones controlled by the PTAPI provider)
// to the gateway, methods on this interface will be called starting
// with <I>reserveLine</I>.  However the gateway may want to implement a
// PtTerminalConnectionListener to register on the associated PtTerminal
// as well, to get call state information on calls to and from the gateway.
// <BR>
// For outgoing calls (to other phones controlled by the PTAPI provider)
// from the gateway, the gateway creates a call via PtProvider::createCall
// then sets up a call via the PtCall::connect method.  The provider service
// will make subsequent calls to the methods in this class starting with
// <I>reserveLine</I>.
//  <BR>
// Calls may be terminated by the gateway via the PtConnection::disconnect
// method, regardless of which side initiated the call.

class PtGatewayInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum PtGatewayAvailability
    {
        UNKNOWN,
        LINE_AVAILABLE,
        ADDRESS_BUSY,
        GATEWAY_BUSY
    }
    //: Availability states for the gateway
    //! enumcode: UNKNOWN - error state
    //! enumcode: LINE_AVAILABLE - a line is available on this gateway on which to setup a call.
    //! enumcode: ADDRESS_BUSY - a line is available on this gateway, but the destination address to be called via this gateway is busy.
    //! enumcode:  GATEWAY_BUSY - all lines are currently busy on this gateway.


/* ============================ CREATORS ================================== */

   PtGatewayInterface();
     //:Default constructor

   virtual
   ~PtGatewayInterface();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual PtStatus reserveLine(PtTerminalConnection& rGatewayTerminalConnection,
                                const char* destinationAddress,
                                PtGatewayAvailability& rAvailablity,
                                void*& rReservationHandle) = 0;
     //: Request to reserve a line on the gateway to the given PBX or PSTN
     //: phone
     // The OpenPhone SDK calls this method to reserve a line
     // on this gateway.  The gateway checks that a line is available and
     // that the identified destination phone is not busy.
     // Note: releaseLine is called even when a call to <I>reserveLine</I> indicates
     // that a line is not available.  <I>reserveLine</I> should therefore always
     // return a reservationHandle which is meaningful to releaseLine.
     // This method must be implemented by the
     // gateway or PBX if call setup is to be performed via the API.
     //!param: (in) rGatewayTerminalConnection - the terminal connection on this gateway for the call.
     //!param: (in) destinationAddress - (presently not used)
     //!param: (out) rAvailablity - GATEWAY_BUSY|ADDRESS_BUSY|AVAILABLE
     //!param: (out) rReservationHandle - reservation handle generated by the gateway.  This is used in subsequent method invocations on the gateway to identify which reservation/terminalConnection/line the operation pertains to.  For example the gateway may use the line number if there are a fixed number of lines that the gateway supports.  This value is merely passed back to the gateway.  It is not used outside the methods defined in this class.

   virtual PtStatus getLineCapabilities(void* reservationHandle,
                                        PtMediaCapabilities& rCapabilities,
                                        char rtpReceiveIpAddress[],
                                        int maxAddressLength,
                                        int& rRtpReceivePort) = 0;
     //: Get the codec capabilities for the reserved line
     // This method must be implemented by the
     // gateway or PBX if call setup is to be performed via the API.
     // Note: This method may be called more than once
     // during a single call on the same line to re-negotiate codecs.
     //!param: (in) reservationHandle - handle indicating the line on which to obtain capabilities
     //!param: (out) rCapabilities - reference to capabilities object defining the send and receive encodings allowed on this line
     //!param: (out) rtpReceiveIpAddress - null terminated ip address or DNS name string of the host that the gateway wishes to receive RTP packets for this line.
     //!param: (in) maxAddressLength - the maximum length that <I>rtpReceiveIpAddress</I> may be.
     //!param: (out) rRtpReceivePort - the port that the gateway wishes to receive RTP packets on the host specified in <I>rtpReceiveIpAddress</I>.

   virtual PtStatus startLineRtpSend(void* reservationHandle, const PtAudioCodec& rSendCodec,
                                    const char* sendAddress, int sendPort) = 0;
     //: Start sending audio via RTP and RTCP for the line indicated
     // This method must be implemented by the
     // gateway or PBX if call setup is to be performed via the API.
     // Note: This method may be called more than once
     // during a single call on the same line to change codecs.
     //! param: (in) reservationHandle - handle indicating the line on which to start sending RTP and RTCP
     //! param: (in) rSendCodec - RTP encoding method to be used when sending RTP
     //! param: (in) sendAddress - the IP address or DNS name of the destination host for the RTP and RTCP packets
     //! param: (in) sendPort - the port on which the destination host expects to receive RTP packets.  The host expects to receive RTCP packets on the port: sendPort + 1.

   virtual PtStatus startLineRtpReceive(void* reservationHandle, const PtAudioCodec& rReceiveCodec) = 0;
     //: Start receiving audio via RTP and RTCP for the line indicated
     // This method must be implemented by the
     // gateway or PBX if call setup is to be performed via the API.
     // Note: that although it is specified what encoding method the gateway should
     // expect in received RTP packets, it is recommended for robustness that
     // the gateway handle any of the encodings specified in the capabilities
     // of the getLineCapabilities method.
     // Note: This method may be called more than once
     // during a single call on the same line to change codecs.
     //! param: (in) reservationHandle - handle indicating the line on which to start receiving RTP and RTCP
     //! param: (in) rReceiveCodec - RTP encoding method to be used when receiving RTP

   virtual PtStatus stopLineRtpSend(void* reservationHandle) = 0;
     //: Stop sending RTP and RTCP for the line indicated
     // This method must be implemented by the
     // gateway or PBX if call setup is to be performed via the API.
     //! param: (in) reservationHandle - handle indicating the line on which to stop sending RTP and RTCP

   virtual PtStatus stopLineRtpReceive(void* reservationHandle) = 0;
     //: Stop receiving RTP and RTCP for the line indicated
     // This method must be implemented by the
     // gateway or PBX if call setup is to be performed via the API.
     //! param: (in) reservationHandle - handle indicating the line on which to stop receiving RTP and RTCP

   virtual PtStatus releaseLine(void* reservationHandle) = 0;
     //: Notify the gateway that line is no longer needed.
     // Note: this method is called even when a call to <I>reserveLine</I> indicates
     // that a line is not available.  <I>reserveLine</I> should therefore always
     // return a reservationHandle which is meaningful to releaseLine.
     // This method must be implemented by the
     // gateway or PBX if call setup is to be performed via the API.
     //! param: (in) reservationHandle - handle indicating the line which is no longer used.

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   PtGatewayInterface(const PtGatewayInterface& rPtGatewayInterface);
     //:Copy constructor (disabled)

   PtGatewayInterface& operator=(const PtGatewayInterface& rhs);
     //:Assignment operator (disabled)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtGatewayInterface_h_
