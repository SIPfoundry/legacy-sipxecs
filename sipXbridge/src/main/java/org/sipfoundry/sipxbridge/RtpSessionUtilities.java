/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.text.ParseException;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.header.extensions.ReferencesHeader;
import gov.nist.javax.sip.header.ims.PAssertedIdentityHeader;

import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.address.SipURI;
import javax.sip.header.AcceptHeader;
import javax.sip.header.AuthorizationHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class RtpSessionUtilities {

	private static final Logger logger = Logger
			.getLogger(RtpSessionUtilities.class);

	/**
	 * Forward the re-INVITE to the iTSP. Creates a corresponding client
	 * transaction and forwards the re-INVITE to the ITSP.
	 * 
	 * @param serverTransaction
	 *            - server transaction to forward
	 * @param dialog
	 *            - dialog associated with server transaction.
	 * 
	 * @throws Exception
	 */
	static void forwardReInvite(RtpSession rtpSession,
			ServerTransaction serverTransaction, Dialog dialog)
			throws Exception {
		AuthorizationHeader authorizationHeader = null;
	    /*
		 * If we are re-negotiating media, then use the new session description
		 * of the incoming invite for the call setup attempt.
		 */
	    
		if (serverTransaction != null) {
		    Request request = serverTransaction.getRequest();
			if (request.getContentLength().getContentLength() != 0) {
				SessionDescription inboundSessionDescription = SipUtilities
						.getSessionDescription(request);
				/*
				 * Exclude stray packets set the destination.
				 */
				String ipAddress = SipUtilities.getSessionDescriptionMediaIpAddress(inboundSessionDescription);
				int port  = SipUtilities.getSessionDescriptionMediaPort(inboundSessionDescription);
				rtpSession.getTransmitter().setIpAddressAndPort(ipAddress, port);
				/*
				 * This is our offer that we are forwarding to the other side.
				 * Store it here. Note that setting the session description also
				 * remaps the ports to where we want to listen.
				 */
				rtpSession.getReceiver().setSessionDescription(
						inboundSessionDescription);
			}
	        authorizationHeader = (AuthorizationHeader)request.getHeader(AuthorizationHeader.NAME);  
		}

		/*
		 * Retrieve the session description from the receiver. This will have
		 * the correct addresses already set due to the setSessionDescription 
		 * operation above.
		 */
		SessionDescription outboundSessionDescription = rtpSession
				.getReceiver().getSessionDescription();

		// HACK alert. Some ITSPs do not like sendonly
		String duplexity = SipUtilities
				.getSessionDescriptionMediaAttributeDuplexity(outboundSessionDescription);
		if (duplexity != null && duplexity.equals("sendonly")) {
			SipUtilities.setDuplexity(outboundSessionDescription, "sendrecv");
		}

		DialogContext dat = (DialogContext) dialog
				.getApplicationData();
		BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
		Dialog peerDialog = DialogContext.getPeerDialog(dialog);

		/*
		 * Check if the server side of the dialog is still alive. If not return
		 * an error.
		 */
		if (peerDialog.getState() == DialogState.TERMINATED) {
			Response response = SipUtilities.createResponse(serverTransaction,
					Response.SERVER_INTERNAL_ERROR);
			response.setReasonPhrase("Peer Dialog is Terminated");
			return;
		}

		DialogContext peerDat = DialogContext.get(peerDialog);

		DialogContext.getRtpSession(peerDialog).getReceiver().setSessionDescription(
				outboundSessionDescription);
		
		SipUtilities.incrementSessionVersion(outboundSessionDescription);

		Request newInvite = peerDialog.createRequest(Request.INVITE);

		SipProvider peerProvider = ((DialogExt) peerDialog).getSipProvider();

		/*
		 * Sending request to ITSP - make the addressing global if required.
		 */
		if (peerProvider != Gateway.getLanProvider()) {
		    SipUtilities.addWanAllowHeaders(newInvite);
			/*
			 * Make sure that global addressing is set.
			 */
			logger.debug("peerDat.itspInfo " + peerDat.getItspInfo());
			if (peerDat.getItspInfo() == null
					|| peerDat.getItspInfo().isGlobalAddressingUsed()) {
				if (Gateway.getGlobalAddress() != null) {
					SipUtilities.setGlobalAddresses(newInvite);	        
				} else {
					javax.sip.header.ReasonHeader warning = ProtocolObjects.headerFactory
							.createReasonHeader("SipXbridge", ReasonCode.SIPXBRIDGE_CONFIG_ERROR,
									"Public address of bridge is not known.");
					b2bua.tearDown(warning);
				}
			}
			
			/*
			 * We do not have any password information for the 
			 * ITSP and this is included in the inbound request
			 * so extract it and send it along.
			 */
			Request request = serverTransaction.getRequest();
            if ( (peerDat.getItspInfo() == null || 
                    peerDat.getItspInfo().getPassword() == null ) &&
                   request.getHeader(AuthorizationHeader.NAME) != null ) {
                AuthorizationHeader authHeader = (AuthorizationHeader) 
                        request.getHeader(AuthorizationHeader.NAME);
                newInvite.setHeader(authHeader);
            }

		} else {
		    SipUtilities.addLanAllowHeaders(newInvite);
		}

		newInvite.removeHeader(PAssertedIdentityHeader.NAME);
		newInvite.addHeader(SipUtilities.createReferencesHeader(serverTransaction.getRequest(),ReferencesHeader.CHAIN));
		AcceptHeader acceptHeader = ProtocolObjects.headerFactory
				.createAcceptHeader("application", "sdp");
		newInvite.setHeader(acceptHeader);
		SipUtilities.addWanAllowHeaders(newInvite);
		ContentTypeHeader cth = ProtocolObjects.headerFactory
				.createContentTypeHeader("application", "sdp");
		newInvite.setContent(outboundSessionDescription.toString(), cth);
		if (authorizationHeader != null) {
		    newInvite.setHeader(authorizationHeader);
		}
		
		

		ClientTransaction ctx = ((DialogExt) peerDialog).getSipProvider()
				.getNewClientTransaction(newInvite);
		TransactionContext tad = TransactionContext.attach(ctx,
				Operation.FORWARD_REINVITE);
		tad.setServerTransaction(serverTransaction);
		
		/*
		 * Set up to forward BYE to peer dialog
		 */
		dat.setForwardByeToPeer(true);
		
		/*
		 * Set the dialog pointer of peer back to us.
		 */
		DialogContext peerDialogContext = DialogContext.get(peerDialog);
		peerDialogContext.setPeerDialog(dialog);
		
				

		DialogContext.get(peerDialog).sendReInvite(ctx);
	}

	/**
	 * Remove RTP session hold and possibly send BYE to MOH server.
	 * 
	 * @param dialog
	 */

	static void removeHold(RtpSession rtpSession,
			ServerTransaction serverTransaction, Dialog dialog) {
		try {
			logger.debug("Remove media on hold!");
			SipUtilities.setDuplexity(rtpSession.getReceiver()
					.getSessionDescription(), "sendrecv");
			SipUtilities.incrementSessionVersion(rtpSession.getReceiver()
					.getSessionDescription());
			Request request = serverTransaction.getRequest();
			SessionDescription sd = SipUtilities.getSessionDescription(request);
			rtpSession.getTransmitter().setOnHold(false);
			rtpSession.getTransmitter().setSessionDescription(sd, true);

			DialogContext dat = (DialogContext) dialog
					.getApplicationData();
			BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();

			b2bua.sendByeToMohServer();

		} catch (Exception ex) {
			logger.error("Unexpected exception removing hold", ex);
			throw new SipXbridgeException(ex);
		}
	}

	/**
	 * Put the session on hold. 
	 * Changes the session attributes to recvonly and sets the transmitter state to hold.
	 * 
	 * @rtpSession -- the rtp session to put on hold.
	 */
	static void putOnHold(RtpSession rtpSession) {

		if (logger.isDebugEnabled()) {
			logger.debug("setting media on hold " + rtpSession.toString());
		}
		rtpSession.getTransmitter().setOnHold(true);
		
		SipUtilities.setDuplexity(rtpSession.getReceiver()
				.getSessionDescription(), "recvonly");
		SipUtilities.incrementSessionVersion(rtpSession.getReceiver()
				.getSessionDescription());
	}

	/**
	 * Re-assign session parameters for the given RTP Session. This method is
	 * called when we get a re-INVITE with SDP. In this case, it could be either
	 * a hold request, resume from hold or simply a port remapping operation. If
	 * this is a hold or a resume then we need to forward the request to the
	 * other side. If port remapping, we can just handle the operation locally.
	 * 
	 * @param serverTransaction
	 *            -- server transaction for this request.
	 *
	 * @throws SdpParseException
	 *             -- if there is a problem with the inbound SDP (for example if
	 *             it is unparseable)
	 * 
	 * @throws SipException
	 *             -- if this is hold or remove hold and there was an issue with
	 *             sending bye to MOH server.
	 * 
	 */
	 static RtpSessionOperation reAssignRtpSessionParameters(	
			ServerTransaction serverTransaction) 
	    throws SdpParseException, ParseException, SipException {
		 
	
		Dialog dialog = serverTransaction.getDialog();
		
		Dialog peerDialog  = DialogContext.getPeerDialog(dialog);
		
		Request request = serverTransaction.getRequest();
		
		RtpSession rtpSession = DialogContext.get(dialog).getRtpSession();
		

		if (peerDialog != null) {
			logger.debug("reAssignSessionParameters: dialog = " + dialog
					+ " peerDialog = "
					+ peerDialog + " peerDialog.lastResponse = \n"
					+ DialogContext.get(peerDialog).getLastResponse());
		}
		
		logger.debug("rtpSession.getTransmitter().sessionDescription = " + rtpSession.getTransmitter().getSessionDescription());

		SessionDescription sessionDescription = SipUtilities.getSessionDescription(request);

		int newport = SipUtilities
				.getSessionDescriptionMediaPort(sessionDescription);
		String newIpAddress = SipUtilities
				.getSessionDescriptionMediaIpAddress(sessionDescription);

		/*
		 * Get the a media attribute -- CAUTION - this only takes care of the
		 * one media attribute. Question - what to do when only one media stream is put
		 * on hold?
		 */

		String mediaAttribute = SipUtilities
				.getSessionDescriptionMediaAttributeDuplexity(sessionDescription);

		String sessionAttribute = SipUtilities
				.getSessionDescriptionAttribute(sessionDescription);

		if (logger.isDebugEnabled()) {
			logger.debug("mediaAttribute = " + mediaAttribute
					+ " sessionAttribute = " + sessionAttribute);
		}

		String attribute = sessionAttribute != null ? sessionAttribute
				: mediaAttribute;

		if (rtpSession.isHoldRequest(sessionDescription)) {
			
			putOnHold(rtpSession);
			return RtpSessionOperation.PLACE_HOLD;

		} else if (rtpSession.getTransmitter().isOnHold() && 
				(attribute == null || attribute.equals("sendrecv"))) {
			/*
			 * Somebody is trying to remove the hold.
			 */
			rtpSession.getTransmitter().setSessionDescription(
					sessionDescription, true);
			removeHold(rtpSession, serverTransaction, dialog);
			return RtpSessionOperation.REMOVE_HOLD;
		} else if (!rtpSession.getTransmitter().getIpAddress().equals(
				newIpAddress)
				|| rtpSession.getTransmitter().getPort() != newport) {
			/*
			 * This is just a re-invite where he has changed his port. simply
			 * adjust the ports so we know where the other end expects to get
			 * data.
			 */
			rtpSession.getTransmitter().setSessionDescription(
					sessionDescription, true);

			return RtpSessionOperation.PORT_REMAP;
		} else if (SipUtilities
				.getSessionDescriptionVersion(sessionDescription) == SipUtilities
				.getSessionDescriptionVersion(rtpSession.getTransmitter()
						.getSessionDescription())) {
			/*
			 * Must be a session keepalive. Mark it as a NO-OP.
			 */
			return RtpSessionOperation.NO_OP;
		} else {
			/*
			 * Must be the other side trying to renegotiate codec.
			 */
			rtpSession.getTransmitter().setSessionDescription(
					sessionDescription, true);

			return RtpSessionOperation.CODEC_RENEGOTIATION;
		}

	}
}
