/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogExt;

import java.util.HashSet;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.ServerTransaction;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

/**
 * Utility functions to send various requests and error responses.
 */
public class CallControlUtilities {

	private static Logger logger = Logger.getLogger(CallControlUtilities.class);

	static void sendInternalError(ServerTransaction st, Exception ex) {
		try {
			Request request = st.getRequest();
			Response response = ProtocolObjects.messageFactory.createResponse(
					Response.SERVER_INTERNAL_ERROR, request);
			if (CallControlManager.logger.isDebugEnabled()) {
				String message = "Exception occured at " + ex.getMessage()
						+ " at " + ex.getStackTrace()[0].getFileName() + ":"
						+ ex.getStackTrace()[0].getLineNumber();

				response.setReasonPhrase(message);
			} else {
				response.setReasonPhrase(ex.getCause().getMessage());
			}
			st.sendResponse(response);

		} catch (Exception e) {
			throw new RuntimeException("Check gateway configuration", e);
		}
	}

	static void sendBadRequestError(ServerTransaction st, Exception ex) {
		try {
			Request request = st.getRequest();
			Response response = ProtocolObjects.messageFactory.createResponse(
					Response.BAD_REQUEST, request);
			if (CallControlManager.logger.isDebugEnabled()) {
				String message = "Exception occured at " + ex.getMessage()
						+ " at " + ex.getStackTrace()[0].getFileName() + ":"
						+ ex.getStackTrace()[0].getLineNumber();

				response.setReasonPhrase(message);
			}
			st.sendResponse(response);

		} catch (Exception e) {
			throw new RuntimeException("Check gateway configuration", e);
		}
	}

	/**
	 * Sends an in-dialog SDP Offer to the peer of this dialog.
	 * 
	 * @param response
	 * @param dialog
	 * @throws Exception
	 */
	static void sendSdpReOffer(Response response, Dialog dialog)
			throws Exception {
		//DialogApplicationData dialogContext = (DialogApplicationData) dialog
		//		.getApplicationData();
		BackToBackUserAgent b2bua = DialogContext.getBackToBackUserAgent(dialog);
		DialogContext dialogContext = (DialogContext) dialog
				.getApplicationData();
		if (logger.isDebugEnabled()) {
			logger.debug("sendSdpOffer : peerDialog = " + dialog
					+ " peerDialogApplicationData = " + dialogContext
					+ "\nlastResponse = " + dialogContext.lastResponse);
		}

	
		b2bua.sendByeToMohServer();

		/*
		 * Create a new INVITE to send to the other side. The sdp is extracted
		 * from the response and fixed up.
		 */

		if (response.getContentLength().getContentLength() != 0) {
			/*
			 * Possibly filter the outbound SDP ( if user sets up to do so ).
			 */
			SessionDescription sdpOffer = SipUtilities.cleanSessionDescription(
					SipUtilities.getSessionDescription(response), Gateway
							.getCodecName());

			Request sdpOfferInvite = dialog.createRequest(Request.INVITE);

			/*
			 * Got a Response to our SDP query. Shuffle to the other end.
			 */

			DialogContext.getRtpSession(dialog).getTransmitter()
					.setOnHold(false);

			/*
			 * Set and fix up the sdp offer to send to the opposite side.
			 */
			DialogContext.getRtpSession(dialog).getReceiver()
					.setSessionDescription(sdpOffer);

			SipUtilities.incrementSessionVersion(sdpOffer);

			SipUtilities.fixupOutboundRequest(dialog, sdpOfferInvite);

			sdpOfferInvite.setContent(sdpOffer.toString(),
					ProtocolObjects.headerFactory.createContentTypeHeader(
							"application", "sdp"));

			ClientTransaction ctx = ((DialogExt) dialog).getSipProvider()
					.getNewClientTransaction(sdpOfferInvite);

			TransactionContext.attach(ctx, Operation.SEND_SDP_RE_OFFER);

			dialog.sendRequest(ctx);

		} else {
			dialogContext.getBackToBackUserAgent().tearDown(
					ProtocolObjects.headerFactory.createReasonHeader(
							Gateway.SIPXBRIDGE_USER, ReasonCode.PROTOCOL_ERROR,
							"No SDP in response"));
		}

	}

	/**
	 * Sends an SDP answer to the peer of this dialog.
	 * 
	 * @param response
	 *            - response from which we are going to extract the SDP answer
	 * @param dialog
	 *            -- dialog for the interaction
	 * 
	 * @throws Exception
	 *             - if there was a problem extacting sdp or sending ACK
	 */
	static void sendSdpAnswerInAck(Response response, Dialog dialog)
			throws Exception {

		DialogContext dialogContext = (DialogContext) dialog
				.getApplicationData();
		if (logger.isDebugEnabled()) {
			logger.debug("sendSdpAnswerInAck : dialog = " + dialog
					+ " peerDialogApplicationData = " + dialogContext
					+ "\nlastResponse = " + dialogContext.lastResponse);
		}

		dialogContext.setPendingAction(PendingDialogAction.NONE);
		
		if (response.getContentLength().getContentLength() != 0) {

			SessionDescription answerSessionDescription = SipUtilities
					.cleanSessionDescription(SipUtilities
							.getSessionDescription(response), Gateway
							.getCodecName());
			/*
			 * Get the codecs in the answer.
			 */
			HashSet<Integer> answerCodecs = SipUtilities
					.getCodecNumbers(answerSessionDescription);

			/*
			 * Get the transmitter session description for the peer. This is
			 * either our old answer or our old offer.
			 */
			SessionDescription transmitterSd = DialogContext
					.getPeerTransmitter(dialog).getTransmitter()
					.getSessionDescription();
			/*
			 * Extract the codec numbers previously offered.
			 */
			HashSet<Integer> transmitterCodecs = SipUtilities
					.getCodecNumbers(transmitterSd);

			/*
			 * The session description to send back in the ACK.
			 */
			SessionDescription ackSd = null;
			/*
			 * Could not find a codec match. We do not want to drop the call in
			 * this case, so just fake it and send the original answer back.
			 */

			if (answerCodecs.size() == 0) {

				/*
				 * We did a SDP query. So we need to put an SDP Answer in the
				 * response. Retrieve the previously offered session
				 * description.
				 */

				ackSd = DialogContext.getRtpSession(dialog)
						.getReceiver().getSessionDescription();

				/*
				 * Only pick the codecs that the other side will support.
				 */
				SipUtilities.restictToSpecifiedCodecs(ackSd, transmitterCodecs);

				DialogContext.getRtpSession(dialog).getTransmitter()
						.setOnHold(false);

			} else {
				/*
				 * Got a Response to our SDP offer solicitation. Shuffle to the
				 * other end. Note that we have to carefully replay a paired
				 * down session description from the original offer --
				 * intersecting it with the original offer.
				 */

				ackSd = answerSessionDescription;

				/*
				 * Fix up the ports.
				 */

				DialogContext.getRtpSession(dialog).getReceiver()
						.setSessionDescription(ackSd);

				DialogContext.getRtpSession(dialog).getTransmitter()
						.setOnHold(false);

				SipUtilities.incrementSessionVersion(ackSd);

			}

			/*
			 * HACK ALERT -- some ITSPs look at sendonly and start playing their
			 * own MOH. This hack is to get around that nasty behavior.
			 */
			if (SipUtilities
					.getSessionDescriptionMediaAttributeDuplexity(ackSd) != null
					&& SipUtilities
							.getSessionDescriptionMediaAttributeDuplexity(ackSd)
							.equals("sendonly")) {
				SipUtilities.setDuplexity(ackSd, "sendrecv");
			}
			Request ackRequest = dialog.createAck(SipUtilities
					.getSeqNumber(dialogContext.lastResponse));
			/*
			 * Consume the last response.
			 */
			dialogContext.lastResponse = null;

			/*
			 * Answer is no longer pending.
			 */
			dialogContext.setPendingAction(PendingDialogAction.NONE);

			/*
			 * Send the SDP answer in an ACK.
			 */

			ackRequest.setContent(ackSd.toString(),
					ProtocolObjects.headerFactory.createContentTypeHeader(
							"application", "sdp"));
			dialog.sendAck(ackRequest);

		} else {
			logger.error("ERROR  0 contentLength ");
		}

	}

}
