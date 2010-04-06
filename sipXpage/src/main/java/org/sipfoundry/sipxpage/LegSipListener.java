/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.net.InetSocketAddress;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Vector;
import java.util.concurrent.ConcurrentHashMap;

import javax.sdp.Attribute;
import javax.sdp.Connection;
import javax.sdp.Media;
import javax.sdp.MediaDescription;
import javax.sdp.SdpConstants;
import javax.sdp.SdpException;
import javax.sdp.SdpFactory;
import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.InvalidArgumentException;
import javax.sip.ListeningPoint;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipFactory;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.SipStack;
import javax.sip.TimeoutEvent;
import javax.sip.Transaction;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.address.Address;
import javax.sip.address.AddressFactory;
import javax.sip.address.SipURI;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.CallInfoHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentLengthHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.Header;
import javax.sip.header.HeaderFactory;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.UserAgentHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Message;
import javax.sip.message.MessageFactory;
import javax.sip.message.Request;
import javax.sip.message.Response;
import gov.nist.javax.sip.message.SIPRequest;
import gov.nist.javax.sip.address.SipUri;
import org.apache.log4j.Logger;

/**
 *
 * The main class that deals with the NIST SIP stack, handling
 * the details of what the SIP stack desires, and in turn generating
 * LegEvents to the various legs and other LegListeners.
 *
 * @author Woof!
 *
 */
public class LegSipListener implements SipListener
{
   static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxpage") ;
   SipProvider sipProvider ;
   SipStack sipStack ;
   SipFactory sipFactory ;
   MessageFactory messageFactory ;
   AddressFactory addressFactory ;
   HeaderFactory headerFactory ;
   ListeningPoint udpListeningPoint;

   String fromIPAddress ;
   String fromSipAddress ;
   UserAgentHeader userAgent  ;
   LegListener inviteListener ;        // The object we tell about new INVITE messages

   ConcurrentHashMap<Dialog, Leg> dialogLegMap ;
   ConcurrentHashMap<Leg, Dialog> legDialogMap ;

   static final String PCMU = Integer.toString(SdpConstants.PCMU) ;

   public void init(SipFactory sipFactory, SipProvider sipProvider, LegListener inviteListener) throws Exception
   {
      this.sipProvider = sipProvider;
      this.sipFactory = sipFactory;
      this.inviteListener = inviteListener ;

      sipProvider.addSipListener(this);
      sipStack = sipProvider.getSipStack() ;
      udpListeningPoint = sipProvider.getListeningPoint(ListeningPoint.UDP);
      fromIPAddress = udpListeningPoint.getIPAddress() ;
      fromSipAddress = fromIPAddress + ":" + udpListeningPoint.getPort() ;

      messageFactory = sipFactory.createMessageFactory();
      headerFactory = sipFactory.createHeaderFactory() ;
      addressFactory = sipFactory.createAddressFactory() ;

      // Create a UserAgent Header
      ArrayList<String> uaList = new ArrayList<String>() ;
      uaList.add("sipXpage/1.0") ;
      userAgent = headerFactory.createUserAgentHeader(uaList) ;

      // Create a mapping between dialogs and legs
      dialogLegMap = new ConcurrentHashMap<Dialog, Leg>(50) ;
      legDialogMap = new ConcurrentHashMap<Leg, Dialog>(50) ;
   }

   void triggerLegEvent(LegEvent legEvent)
   {
      LegListener legListener = legEvent.getLeg() ;
      if (legListener != null)
      {
         legListener.onEvent(legEvent) ;
      }
      else
      {
         LOG.warn(String.format("LegSipListener::triggerLegEvent HEY!  leg null LegEvent(%s)!", legEvent.getDescription()));
      }
   }

   void triggerLegEvent(Dialog dialog, String eventDescription)
   {
      if (dialog != null)
      {
         Leg leg = dialogLegMap.get(dialog) ;
         if (leg != null)
         {
            LegEvent legEvent = new LegEvent(leg, eventDescription) ;
            triggerLegEvent(legEvent) ;
         }
         else
         {
            LOG.warn(String.format("LegSipListener::triggerLegEvent HEY!  dialog %s not found in dialogLegMap!", dialog.toString()));
         }
      }
   }


   /**
    * Place an outbound call.
    *
    * @param leg
    * @param toAddress The destination of the call
    * @param displayName The display name to use in the From.
    * @param fromCallId The call Id from the page originator request.
    * @param sdp The session description to use in placing the call.
    * @param alertInfoKey  The magic value needed to trigger Auto-Answer on Polycom Phones
    * @return The call Id of the created call
    */
   public String placeCall(Leg leg, SipURI toAddress, String displayName, String fromCallId, SessionDescription sdp, String alertInfoKey) throws Throwable
   {
      // TODO Lookup the toAddress in the registration database, so as to only
      // send to the registered phones.
      LOG.info(String.format("LegSipListener::placeCall to %s", toAddress.toString())) ;
      Request request = buildInviteRequest(leg, displayName, fromCallId, toAddress, alertInfoKey);

      // Create ContentTypeHeader
      ContentTypeHeader contentTypeHeader = headerFactory
            .createContentTypeHeader("application", "sdp");

      request.setContent(sdp, contentTypeHeader) ;


      // Create a client transaction, find the associated dialog
      ClientTransaction inviteTid = sipProvider.getNewClientTransaction(request);
      Dialog dialog = inviteTid.getDialog();

      // Associate the dialog with the leg
      dialogLegMap.put(dialog, leg) ;
      legDialogMap.put(leg, dialog) ;
      LOG.debug(String.format("LegSipListener::placeCall associating dialog(%s) with leg(%s)", dialog.toString(), leg.toString())) ;

      // Associate the invite transaction with the leg (for later cancel)
      leg.setInviteTransaction(inviteTid, false) ;
      inviteTid.setApplicationData(leg) ;

      // Set the name
      leg.setDisplayName(displayName) ;

      // send the request out.
      inviteTid.sendRequest();

      return dialog.getCallId().toString() ;
   }

   public void endCall(Leg leg) throws Exception
   {
      LOG.info(String.format("LegSipListener::endCall Leg %s", leg.toString())) ;
      Dialog dialog = legDialogMap.get(leg) ;

      if (dialog == null)
      {
         LOG.debug(String.format("LegSipListener::endCall Hmm, dialog not found for leg %s%n", leg)) ;
         return ;
      }

      // Invite tx still exists, send a CANCEL or 4xx
      Transaction origInviteTransaction = leg.getInviteTransaction() ;
      if (origInviteTransaction != null)
      {
    	 LOG.info(String.format("LegSipListener::endCall Leg %s origInviteTransaction exists", leg.toString())) ;
         if (leg.isServer())
         {
            // Send a Busy Here response
            ServerTransaction t = (ServerTransaction)origInviteTransaction ;
            Response response = messageFactory.createResponse(Response.BUSY_HERE, t.getRequest()) ;
            // Send it (in Transaction)
            t.sendResponse(response) ;
         }
         else
         {
            // Build a CANCEL request
            ClientTransaction t = (ClientTransaction)origInviteTransaction ;
            Request request = t.createCancel() ;
            request.addHeader(userAgent);
            // Create a new client transaction
            ClientTransaction cancelTransaction = sipProvider.getNewClientTransaction(request);
            // Send it
            cancelTransaction.sendRequest() ;
         }
         return ;
      }

      DialogState dialogState = dialog.getState() ;
      // Already terminated, call is ended.
      if (dialogState == null || dialogState == DialogState.TERMINATED)
      {
    	 LOG.info(String.format("LegSipListener::endCall Leg %s dialogState null or TERMINATED", leg.toString())) ;
         return ;
      }
      else
      {
         if (leg.isByed()) {
             LOG.info(String.format("LegSipListener::endcall Leg %s got a bye, so don't send one", leg.toString()));
         } else {
         	 LOG.info(String.format("LegSipListener::endCall Leg %s dialog %s needs a BYE", leg.toString(), dialog.toString())) ;
             // Build a BYE request
             Request request = dialog.createRequest(Request.BYE) ;
             request.addHeader(userAgent);
             // Create a new client transaction
             ClientTransaction byeTransaction = sipProvider.getNewClientTransaction(request);
             // Send it (in Dialog)
             dialog.sendRequest(byeTransaction) ;
         }
      }
   }

   public void acceptCall(Leg leg) throws Throwable
   {
      LOG.info(String.format("LegSipListener::acceptCall Leg %s", leg.toString())) ;

      Transaction transactionId = leg.getInviteTransaction() ;
      ServerTransaction serverTransactionId = (ServerTransaction)transactionId ;

      // Clear the invite transaction of the leg
      leg.setInviteTransaction(null, false) ;


      Request request = serverTransactionId.getRequest() ;

      FromHeader fromHeader = (FromHeader)request.getHeader(FromHeader.NAME) ;
      String displayName = fromHeader.getAddress().getDisplayName() ;
      if (displayName == null)
      {
         displayName = fromHeader.getAddress().getURI().toString() ;
      }
      leg.setDisplayName(displayName) ;

      InetSocketAddress localRtpAddress = new
         InetSocketAddress(udpListeningPoint.getIPAddress(), leg.getRtpPort()) ;
      SessionDescription sdp = buildSdp(localRtpAddress, leg.getRtpPort() == 0) ;
      Response response = messageFactory.createResponse(Response.OK, request) ;

      // Add Leg's tag
      ToHeader toHeader = (ToHeader)response.getHeader(ToHeader.NAME);
      toHeader.setTag(leg.getTag()) ;

      // Create contact headers
      SipURI fromURI = addressFactory.createSipURI("pager", fromSipAddress);
      SipURI contactUrl = fromURI;
      contactUrl.setPort(udpListeningPoint.getPort());


      // Create the contact name address.
      Address contactAddress = addressFactory.createAddress(fromURI);
      ContactHeader contactHeader = headerFactory.createContactHeader(contactAddress);
      response.addHeader(contactHeader);

      // Create ContentTypeHeader
      ContentTypeHeader contentTypeHeader = headerFactory
            .createContentTypeHeader("application", "sdp");

      // Add the sdp
      response.setContent(sdp, contentTypeHeader) ;
      sendServerResponse(serverTransactionId, response);
   }

   /**
    * Build an INVITE request to the paged phone, with various headers needed
    * to trigger Auto-Answer on Polycom, SNOM, and other phones.
    *
    * <p>
    * Adds an Alert-info header if alertInfoKey is not null.
    *    Polycom uses this.
    * </p><p>
    * Adds a Call-info header with <phone>;answer-after=0
    *    SNOM and others use this.
    * </p>
    *
    * @param leg The leg that will be sending this INVITE (uses it's tag for "from")
    * @param fromDisplayName The text to be used in the From display
    * @param toAddress The request URI being INVITEd
    * @param alertInfoKey The magic value needed to trigger Auto-Answer on Polycom Phones
    * @return The INVITE request
    * @throws Exception
    */
   Request buildInviteRequest(Leg leg, String fromDisplayName, String fromCallId, SipURI toAddress, String alertInfoKey) throws Throwable
   {
      // create From Header
      SipURI fromURI = addressFactory.createSipURI("pager", fromSipAddress);
      Address fromNameAddress = addressFactory.createAddress("sip:pager@"+fromSipAddress) ;
      fromNameAddress.setDisplayName(fromDisplayName);
      FromHeader fromHeader = headerFactory.createFromHeader(fromNameAddress,
            leg.getTag()); // Use leg's tag

      // create To Header
      Address toNameAddress = addressFactory.createAddress(toAddress);
      ToHeader toHeader = headerFactory.createToHeader(toNameAddress, null);

      // create Request URI
      SipURI requestURI = toAddress;

      // Add the sipx-noroute=VoiceMail and sipx-userforward=false parameters
      requestURI.setParameter("sipx-noroute", "VoiceMail") ;
      requestURI.setParameter("sipx-userforward", "false") ;

      // Create ViaHeaders

      ArrayList<ViaHeader> viaHeaders = new ArrayList<ViaHeader>();
      ViaHeader viaHeader = headerFactory.createViaHeader(udpListeningPoint.getIPAddress(),
            udpListeningPoint.getPort(),
            ListeningPoint.UDP, null);

      // add via headers
      viaHeaders.add(viaHeader) ;

      // Create a new CallId header
      CallIdHeader callIdHeader = sipProvider.getNewCallId();

      // Create a new Cseq header
      CSeqHeader cSeqHeader = headerFactory.createCSeqHeader(1L,
            Request.INVITE);

      // Create a new MaxForwards Header
      MaxForwardsHeader maxForwards = headerFactory
            .createMaxForwardsHeader(70);

      // Create the request.
      Request request = messageFactory.createRequest(requestURI,
            Request.INVITE, callIdHeader, cSeqHeader, fromHeader,
            toHeader, viaHeaders, maxForwards);

      // Add user agent
      request.addHeader(userAgent);

      if (alertInfoKey != null)
      {
         // Add Alert-Info header (Cannot use headerFactory.createAlertInfoHeader(URI), so
         // just use a generic header
         Header alertInfo = headerFactory.createHeader("Alert-info", alertInfoKey) ;
         request.addHeader(alertInfo) ;
      }

      // Add Call-Info header
      CallInfoHeader callInfo = headerFactory.createCallInfoHeader(toAddress) ;
      callInfo.setParameter("answer-after", "0") ;
      request.addHeader(callInfo) ;

      // Create contact headers
      SipURI contactUrl = fromURI;
      contactUrl.setPort(udpListeningPoint.getPort());
      contactUrl.setLrParam();


      // Create the contact name address.
      Address contactAddress = addressFactory.createAddress(fromURI);

      ContactHeader contactHeader = headerFactory.createContactHeader(contactAddress);
      request.addHeader(contactHeader);

      // Create the Reference header.
      if (fromCallId != null)
      {
          Header referencesHeader = headerFactory.createHeader("References", fromCallId + ";rel=chain") ;
          request.addHeader(referencesHeader);
      }


      return request ;
   }


   /**
    * Build the Session Description corresponding to an RTP address
    * (Assuming uLaw, ptime=20 mS, AVP=audio)
    *
    * @param localRtpAddress IP and Port of the desired SDP
    * @param sendOnly True if this SDP is to be marked "sendonly".  If so, localRtpAddress
    * is not used.
    * @return The Session Description
    * @throws Exception
    */
   public SessionDescription buildSdp(InetSocketAddress localRtpAddress, boolean sendOnly) throws Throwable
   {
      String localHost = "0.0.0.0" ;
      int localPort = 0 ;
      Vector <Attribute>attrs = new Vector<Attribute>();
      if (!sendOnly)
      {
         // Get the dotted quad IP address, not the name.  SDP doesn't want to do DNS
         // queries to resolve IP addresses
         localHost = localRtpAddress.getAddress().getHostAddress() ;
         localPort = localRtpAddress.getPort() ;
      }
      SdpFactory sdpFactory = SdpFactory.getInstance() ;
      SessionDescription sdp = sdpFactory.createSessionDescription() ;
      sdp.setOrigin(sdpFactory.createOrigin("pager", 42, 42, "IN", "IP4", localHost)) ;
      sdp.setSessionName(sdpFactory.createSessionName("pager")) ;
      sdp.setConnection(sdpFactory.createConnection(localHost));
      Vector <MediaDescription>mediaList = new Vector<MediaDescription>() ;
      String[] codecs = {PCMU};
      attrs.add(sdpFactory.createAttribute("rtpmap", "0 PCMU/8000"));
      attrs.add(sdpFactory.createAttribute("ptime", "20"));
      if (sendOnly)
      {
         attrs.add(sdpFactory.createAttribute("sendonly", null)) ;
      }
      else
      {
         attrs.add(sdpFactory.createAttribute("sendrecv", null)) ;
      }
      MediaDescription md = sdpFactory.createMediaDescription("audio", localPort, 2, "RTP/AVP", codecs);
      md.setAttributes(attrs);
      mediaList.add(md);
      sdp.setMediaDescriptions(mediaList) ;

      return sdp ;
   }

   Response processInvite(Request request, ServerTransaction serverTransactionId) throws Throwable
   {
      Dialog dialog = null ;


      if (serverTransactionId != null)
      {
         // An existing dialog, it must be a re-invite
         dialog = serverTransactionId.getDialog() ;
         LOG.info(String.format("LegSipListener::processInvite re-Invite dialog=(%s)", dialog)) ;

         Leg leg = (Leg)serverTransactionId.getApplicationData() ;
         if (leg == null)
         {
            leg = dialogLegMap.get(dialog) ;
            serverTransactionId.setApplicationData(leg) ;
            leg.setInviteTransaction(serverTransactionId, true) ;
         }

         InetSocketAddress addressPort = handleSdp(request) ;
         if (addressPort != null)
         {
            // Tell Leg about sdp
            LegEvent legEvent = new LegEvent(leg, "sdp");
            legEvent.setSdpAddress(addressPort);
            triggerLegEvent(legEvent);
         }
         else
         {
            triggerLegEvent(dialog, "invite dialog reinvite recieved from far end (no sdp)") ;
         }
         acceptCall(leg) ; // Send the 200 OK
      }
      else
      {
         // A new call

         LOG.info("LegSipListener::processInvite new Invite") ;

         // Create a new transaction
         serverTransactionId = sipProvider.getNewServerTransaction(request);
         dialog = serverTransactionId.getDialog() ;

         // Create the leg and mappings
         InboundLeg leg = new InboundLeg(this, inviteListener) ;
         SIPRequest sipReq = (SIPRequest) request;
         FromHeader fromHeader = (FromHeader)sipReq.getHeader(FromHeader.NAME);
         if ( fromHeader != null)
         {
            if( fromHeader.getAddress().getURI() instanceof SipUri )
            {
               SipUri sipUri = (SipUri) fromHeader.getAddress().getURI();
               leg.setAddress( sipUri.getUserAtHostPort() );
            }
         }
         leg.setCallId(dialog.getCallId().getCallId());
         serverTransactionId.setApplicationData(leg) ;
         dialogLegMap.put(dialog, leg) ;
         legDialogMap.put(leg, dialog) ;
         LOG.debug(String.format("LegSipListener::processInvite associating dialog(%s) with leg(%s)", dialog.toString(), leg.toString())) ;

         // Tell our invite listener
         LegEvent legEvent = new LegEvent(leg, "invite") ;
         legEvent.setSdpAddress(handleSdp(request)) ;
         leg.setInviteTransaction(serverTransactionId, true) ;
         inviteListener.onEvent(legEvent) ;
      }
      return null ;  // No response.  Let the appropriate LegListener handle that.
   }

   Response processBye(Request request, ServerTransaction serverTransactionId) throws ParseException
   {
      Dialog dialog = serverTransactionId.getDialog() ;
      triggerLegEvent(dialog, "dialog bye recieved from far end") ;
      return messageFactory.createResponse(Response.OK, request) ;
   }

   Response processOptions(Request request, ServerTransaction serverTransactionId) throws ParseException
   {
      return messageFactory.createResponse(Response.ACCEPTED,request) ;
   }

   public void processDialogTerminated(DialogTerminatedEvent dialogTerminatedReceivedEvent)
   {
      Dialog dialog = dialogTerminatedReceivedEvent.getDialog() ;
      LOG.info(String.format("LegSipListener Dialog Terminated Event on dialog id " + dialog.getDialogId()));

      // Tell the leg goodbye and remove the mapping
      triggerLegEvent(dialog, "dialog terminated") ;
      Leg leg = dialogLegMap.remove(dialog) ;
      if (leg != null) {
    	 legDialogMap.remove(leg) ;
         LOG.debug(String.format("LegSipListener::processDialogTerminated removing dialog(%s) with leg(%s)", dialog.toString(), leg.toString())) ;
      }
   }

   public void processIOException(IOExceptionEvent arg0)
   {
      LOG.warn(String.format("LegSipListener::processIOException Event " + arg0.toString()));
   }

   public void processRequest(RequestEvent requestReceivedEvent) {
      Request request = requestReceivedEvent.getRequest();
      ServerTransaction serverTransactionId = requestReceivedEvent
            .getServerTransaction();
      Leg leg = serverTransactionId != null ? (Leg)serverTransactionId.getApplicationData() : null ;

      LOG.info(String.format("LegSipListener::processRequest %s received at %s",
            request.getMethod(), sipStack.getStackName())) ;

      LOG.info(String.format("LegSipListener::processRequest transactionId %s Leg %s",
         serverTransactionId != null ? serverTransactionId.toString() : "(null)",
         leg != null ? leg.toString() : "(null)")) ;

      Response response = null ;
      try
      {
         String method = request.getMethod() ;
         if (method.equals(Request.BYE))
         {
            response = processBye(request, serverTransactionId);
         }
         else if (method.equals(Request.OPTIONS))
         {
            response = processOptions(request, serverTransactionId);
         }
         else if (method.equals(Request.ACK))
         {
            // Nothing to do here.
         }
         else if (method.equals(Request.INVITE))
         {
            response = processInvite(request, serverTransactionId);
         }
         else
         {
            response = messageFactory.createResponse(Response.BAD_REQUEST,request) ;
         }
      } catch (Throwable t)
      {
         LOG.fatal("LegSipListener::processRequest", t) ;
         System.exit(1) ;
      }

      if (response != null)
      {
         sendServerResponse(serverTransactionId, response);
      }
   }

   public void sendServerResponse(ServerTransaction transactionId, Response response)
   {
      if (response != null)
      {
    	 if (response.getHeader(UserAgentHeader.NAME) == null) {
             response.addHeader(userAgent);
    	 }

    	 try
         {
            if (transactionId != null)
            {
               transactionId.sendResponse(response);
            }
            else
            {
               sipProvider.sendResponse(response) ;
            }
         } catch (Throwable t)
         {
            LOG.warn("LegSipListener::sendServerResponse", t) ;
         }
      }
   }


   public void processResponse(ResponseEvent responseReceivedEvent) {
		Response response = responseReceivedEvent.getResponse();
		CSeqHeader cseq = (CSeqHeader) response.getHeader(CSeqHeader.NAME);
		ClientTransaction clientTransactionId = responseReceivedEvent
				.getClientTransaction();
		Dialog transactionDialog = null;
		Dialog eventDialog = responseReceivedEvent.getDialog() ;

		if (clientTransactionId != null) {
			transactionDialog = clientTransactionId.getDialog();
		}

		LOG.info("LegSipListener::processResponse " + response.getReasonPhrase()
				+ " received at " + sipStack.getStackName()
				+ " with client transaction id "+ clientTransactionId
				+ " tx dialog " + transactionDialog
				+ " event dialog " + eventDialog);

		// Send an ACK for all INVIITE 200 OKs, even if we don't care about them.
		// (Stops the other side from sending more)
		if (response.getStatusCode() == Response.OK
				&& cseq.getMethod().equals(Request.INVITE)) {
			try {
				Request ackRequest = eventDialog.createAck(cseq.getSeqNumber());
				// Add user agent
				ackRequest.addHeader(userAgent);


				// If the eventDialog is not the transactionDialog, this 200 OK is from a fork.
				if (transactionDialog == null || !eventDialog.equals(transactionDialog)) {
					// Send ACK in statelessly so as NOT to accept it
					sipProvider.sendRequest(ackRequest);

					// Send a BYE, we cannot handle multiple forks.
					LOG.debug("LegSipListener::processResponse second 200 OK (forked INVITE?), send BYE");

					// Build a BYE request
					Request byeRequest;
					byeRequest = eventDialog.createRequest(Request.BYE);
					// Add user agent
					byeRequest.addHeader(userAgent);
					// Create a new client transaction
					ClientTransaction byeTransaction = sipProvider
							.getNewClientTransaction(byeRequest);
					// Send it (in Dialog)
					eventDialog.sendRequest(byeTransaction);

					return ;
				} else {
					// Send ACK in Dialog to accept it
					eventDialog.sendAck(ackRequest);
				}

			} catch (InvalidArgumentException e) {
				LOG.warn("LegSipListener::processResponse", e);
				return;
			} catch (SipException e) {
				LOG.warn("LegSipListener::processResponse", e);
				return;
			}
		}

		if (transactionDialog != null) {
			// Deal with SDP in any response
			InetSocketAddress addressPort = handleSdp(response);
			if (addressPort != null) {
				// Tell Leg about SDP
				LegEvent legEvent = new LegEvent(dialogLegMap.get(transactionDialog),
						"sdp");
				legEvent.setSdpAddress(addressPort);
				triggerLegEvent(legEvent);
			}

			if (cseq.getMethod().equals(Request.INVITE)) {
				// Ignore 1XX responses
				if (response.getStatusCode() < Response.OK)
					return;

				Leg leg = (Leg) clientTransactionId.getApplicationData();

				if (response.getStatusCode() == Response.OK) {
					if (leg != null && leg.getInviteTransaction() != null) {
						// The invite transaction is completed.
						triggerLegEvent(transactionDialog, "dialog connected");
					}
				} else {
					triggerLegEvent(transactionDialog, "dialog failed to connect");
				}
				if (leg != null) {
					// The invite transaction is completed.
					leg.setInviteTransaction(null, false);
				}
			}
		}
	}

   /**
	 * Given a SIP message, find any SDP embedded in it and return the IP
	 * address and port number that represents uLaw audio.
	 *
	 * @param message
	 * @return The InetSocketAddress or null if nothing found
	 */
   InetSocketAddress handleSdp(Message message)
   {
      // TODO Handle multi-part MIME
      // Check for SDP
      ContentLengthHeader contentLengthHeader = message.getContentLength() ;
      if (contentLengthHeader != null && contentLengthHeader.getContentLength() > 0)
      {
         try
         {
            String rawContent = new String(message.getRawContent()) ;
            SessionDescription sdp = SdpFactory.getInstance().createSessionDescription(rawContent) ;
            if (sdp != null)
            {
               Vector mediaDescriptions = sdp.getMediaDescriptions(false);
               // Find the ip:port for the uLaw audio codec
               for (Iterator iter = mediaDescriptions.iterator(); iter.hasNext();)
               {
                  MediaDescription md = (MediaDescription) iter.next();

                  Media media = md.getMedia() ;
                  if (media.getMediaType().compareToIgnoreCase("audio") == 0)
                  {
                     Vector formats = media.getMediaFormats(false) ;
                     for (Iterator iter2 = formats.iterator(); iter2.hasNext();)
                     {
                        String format = (String) iter2.next();
                        if (format.equals(PCMU))
                        {
                           // Check the connection with the media
                           Connection c = md.getConnection() ;
                           if (c == null)
                           {
                              // None found, get the global connection
                              c = sdp.getConnection();
                           }
                           String address = c.getAddress() ;
                           int port = media.getMediaPort() ;
                           InetSocketAddress addressPort = new InetSocketAddress(
                                 address, port);
                           return addressPort ;

                        }
                     }
                  }
               }
            }
         } catch (SdpException e)
         {
            // Guess the content wasn't sdp (or not valid sdp).  Ignore it then
         }
      }
      return null ;
   }

   public void processTimeout(TimeoutEvent arg0)
   {
      // TODO What to do here?
      LOG.warn("LegSipListener::processTimeout got "+arg0) ;
   }

   public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedReceivedEvent)
   {
      Transaction tid ;
      String ua = "dunno" ;

      if (transactionTerminatedReceivedEvent.isServerTransaction())
      {
         ua = "Server" ;
         tid = transactionTerminatedReceivedEvent.getServerTransaction() ;
      }
      else
      {
         ua = "Client" ;
         tid = transactionTerminatedReceivedEvent.getClientTransaction() ;
      }
      LOG.info(String.format("LegSipListener::processTransactionTerminated Event on %s transaction %s id %s", ua,
            tid.getRequest().getMethod(), tid.toString()));

      // Clear this transaction from the leg
      Leg leg = (Leg)tid.getApplicationData() ;
      if (leg != null)
      {
         leg.setInviteTransaction(null, false) ;
      }
   }
}
