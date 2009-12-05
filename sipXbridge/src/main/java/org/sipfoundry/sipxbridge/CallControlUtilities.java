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

import java.util.Set;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.InvalidArgumentException;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
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
            if (CallControlManager.logger.isInfoEnabled()) {
                String message = "Exception Info " + ex.getMessage() + " at "
                        + ex.getStackTrace()[0].getFileName() + ":"
                        + ex.getStackTrace()[0].getLineNumber();
                SipUtilities.addSipFrag(response,message);
               
            } 
            st.sendResponse(response);

        } catch (Exception e) {
            logger.error("Check gateway configuration", e);
        }
    }

    static void sendBadRequestError(ServerTransaction st, Exception ex) {
        try {

            Response response = SipUtilities.createResponse(st, Response.BAD_REQUEST);
            if (CallControlManager.logger.isInfoEnabled()) {
                String message = "Exception Info " + ex.getMessage() + " at "
                        + ex.getStackTrace()[0].getFileName() + ":"
                        + ex.getStackTrace()[0].getLineNumber();
               
                SipUtilities.addSipFrag(response,message);
        
            }
            st.sendResponse(response);

        } catch (Exception e) {
            logger.error("Check gateway configuration", e);
        }
    }
    
    
    public static void sendServiceUnavailableError(
			ServerTransaction serverTransaction, Exception ex) {
	
    	 try {

             Response response = SipUtilities.createResponse(serverTransaction, Response.SERVICE_UNAVAILABLE);
             if (CallControlManager.logger.isInfoEnabled()) {        	 
                 String message = "Exception Info " + ex.getMessage() + " at "
                         + ex.getStackTrace()[0].getFileName() + ":"
                         + ex.getStackTrace()[0].getLineNumber();
                 SipUtilities.addSipFrag(response,message);
                 response.setReasonPhrase(ex.getMessage());
             } else {
            	response.setReasonPhrase(ex.getMessage());
             }
             serverTransaction.sendResponse(response);

         } catch (Exception e) {
             logger.error("Check gateway configuration", e);
         }
	}
    
    /**
     * Sends an in-dialog SDP Offer to the peer of this dialog.
     * 
     * @param response
     * @param reOfferDialog
     * @throws Exception
     */
    static void sendSdpReOffer(Response response, Dialog responseDialog, Dialog reOfferDialog) throws Exception {
    	DialogContext.pairDialogs(responseDialog, reOfferDialog);
        BackToBackUserAgent b2bua = DialogContext.getBackToBackUserAgent(reOfferDialog);
        DialogContext dialogContext = (DialogContext) reOfferDialog.getApplicationData();
        if (logger.isDebugEnabled()) {
            logger.debug("sendSdpOffer : peerDialog = " + reOfferDialog
            		+ " dialogState = " + reOfferDialog.getState() 
                    + " peerDialogApplicationData = " + dialogContext + "\nlastResponse = "
                    + dialogContext.getLastResponse());
        }

        if (b2bua.getMusicOnHoldDialog() != null && 
                b2bua.getMusicOnHoldDialog().getState() != DialogState.TERMINATED ) {
         /*
          * If MOH is still alive, kill and retry. We put a small pause here to shut the MOH media down
          * so any packets can be flushed before media re-negotiation takes place.
          */
          b2bua.sendByeToMohServer();
            
           /*
            * Check to see if the MOH dialog will be terminated when it is confirmed. If so, we do not
            * have to pause here because the MOH has not been streamed to the ITSP. Otherwise, put in a delay
            * so any packets bound for the ITSP have cleared before we re-negotiate media.
            */
            if (!DialogContext.get(b2bua.getMusicOnHoldDialog()).isTerminateOnConfirm()) {
            	Gateway.getTimer().schedule(new ReOfferTimerTask(response,responseDialog,reOfferDialog), 50);
            	return;
            }
        }
        

        /*
         * Create a new INVITE to send to the other side. The sdp is extracted from the response
         * and fixed up.
         */

        if (response.getContentLength().getContentLength() != 0) {
            /*
             * Get the session description from the response.
             */
            SessionDescription sdpOffer = SipUtilities.getSessionDescription(response);

           
            /*
             * Got a Response to our SDP query. Shuffle to the other end.
             */

            DialogContext.getRtpSession(reOfferDialog).getTransmitter().setOnHold(false);
            
            
            DialogContext.get(reOfferDialog).sendSdpReOffer(sdpOffer);   

        } else {
            dialogContext.getBackToBackUserAgent().tearDown(
                    ProtocolObjects.headerFactory.createReasonHeader(Gateway.SIPXBRIDGE_USER,
                            ReasonCode.PROTOCOL_ERROR, "No SDP in response"));
        }

    }

    /**
     * Sends an SDP answer to the peer of this dialog.
     * 
     * @param response - response from which we are going to extract the SDP answer
     * @param dialog -- dialog for the interaction
     * 
     * @throws Exception - if there was a problem extacting sdp or sending ACK
     */
    static void sendSdpAnswerInAck(Response response, Dialog dialog) throws Exception {

        DialogContext dialogContext = (DialogContext) dialog.getApplicationData();
        if (logger.isDebugEnabled()) {
            logger.debug("sendSdpAnswerInAck : dialog = " + dialog
                    + " peerDialogApplicationData = " + dialogContext + "\nlastResponse = "
                    + dialogContext.getLastResponse());
        }

        dialogContext.setPendingAction(PendingDialogAction.NONE);

        if (response.getContentLength().getContentLength() != 0) {

            SessionDescription answerSessionDescription = SipUtilities
                    .getSessionDescription(response);
             
            /*
             * Get the codecs in the answer.
             */
            Set<Integer> answerCodecs = SipUtilities.getMediaFormats(answerSessionDescription);

            /*
             * Get the transmitter session description for the peer. This is either our old answer
             * or our old offer.
             */
            logger.debug("peerTransmitter = " + DialogContext.getPeerTransmitter(dialog));

            /*
             * The session description to send back in the ACK.
             */
            SessionDescription ackSd = null;
            /*
             * Could not find a codec match. We do not want to drop the call in this case, so just
             * fake it and send the original answer back.
             */

            if (answerCodecs.size() == 0) {
                SessionDescription transmitterSd = DialogContext.getPeerTransmitter(dialog)
                        .getTransmitter().getSessionDescription();
                /*
                 * Extract the codec numbers previously offered.
                 */
                Set<Integer> transmitterCodecs = SipUtilities.getMediaFormats(transmitterSd);

                /*
                 * We did a SDP query. So we need to put an SDP Answer in the response. Retrieve
                 * the previously offered session description.
                 */

                ackSd = DialogContext.getRtpSession(dialog).getReceiver().getSessionDescription();

                /*
                 * Only pick the codecs that the other side will support.
                 */
                SipUtilities.cleanSessionDescription(ackSd, transmitterCodecs);

                DialogContext.getRtpSession(dialog).getTransmitter().setOnHold(false);

            } else {
                /*
                 * Set the remote port so that we know where to expect media from.
                 */
                 ackSd = answerSessionDescription;

                /*
                 * Fix up the ports.
                 */

                DialogContext.getRtpSession(dialog).getReceiver().setSessionDescription(ackSd);

                DialogContext.getRtpSession(dialog).getTransmitter().setOnHold(false);

                SipUtilities.incrementSessionVersion(ackSd);

            }

            /*
             * HACK ALERT -- some ITSPs look at sendonly and start playing their own MOH. This
             * hack is to get around that nasty behavior.
             */
            if (SipUtilities.getSessionDescriptionMediaAttributeDuplexity(ackSd) != null
                    && SipUtilities.getSessionDescriptionMediaAttributeDuplexity(ackSd).equals(
                            "sendonly")) {
                SipUtilities.setDuplexity(ackSd, "sendrecv");
            }
            Request ackRequest = dialog.createAck(SipUtilities.getSeqNumber(dialogContext
                    .getLastResponse()));
            /*
             * Consume the last response.
             */
            dialogContext.setLastResponse(null);

           
            /*
             * Send the SDP answer in an ACK.
             */

            ackRequest.setContent(ackSd.toString(), ProtocolObjects.headerFactory
                    .createContentTypeHeader("application", "sdp"));
            dialogContext.sendAck(ackRequest);
            /*
             * Answer is no longer pending.
             */
            dialogContext.setPendingAction(PendingDialogAction.NONE);


        } else {
            logger.error("ERROR  0 contentLength  -- resend the old SDP answer");
            sendSdpAnswerInAck(dialog,null);
        }

    }
    
    
    /**
     * Replay the old SDP offer in the ACK. This condition is encountered when we encounter an error during
     * call transfer.
     * 
     * @param dialog
     */

    static void sendSdpAnswerInAck(Dialog dialog, Set<Integer> codecs) throws Exception {
        
        DialogContext dialogContext = (DialogContext) dialog.getApplicationData();
        if (logger.isDebugEnabled()) {
            logger.debug("sendSdpAnswerInAck : dialog = " + dialog
                    + " peerDialogApplicationData = " + dialogContext + "\nlastResponse = "
                    + dialogContext.getLastResponse());
        }

        dialogContext.setPendingAction(PendingDialogAction.NONE);

        SessionDescription transmitterSd = DialogContext.getPeerTransmitter(dialog)
        .getTransmitter().getSessionDescription();
        /*
         * Extract the codec numbers previously offered.
         */
        Set<Integer> transmitterCodecs = codecs == null ? SipUtilities.getMediaFormats(transmitterSd) : codecs;

        /*
         * We did a SDP query. So we need to put an SDP Answer in the response. Retrieve
         * the previously offered session description.
         */

        SessionDescription ackSd = DialogContext.getRtpSession(dialog).getReceiver().getSessionDescription();

        /*
         * Only pick the codecs that the other side will support.
         */
        SipUtilities.cleanSessionDescription(ackSd, transmitterCodecs);

        DialogContext.getRtpSession(dialog).getTransmitter().setOnHold(false);
        
        Request ackRequest = dialog.createAck(SipUtilities.getSeqNumber(dialogContext
                .getLastResponse()));
        /*
         * Consume the last response.
         */
        dialogContext.setLastResponse(null);

        /*
         * Answer is no longer pending.
         */
        dialogContext.setPendingAction(PendingDialogAction.NONE);

        /*
         * Send the SDP answer in an ACK.
         */

        ackRequest.setContent(ackSd.toString(), ProtocolObjects.headerFactory
                .createContentTypeHeader("application", "sdp"));
        dialogContext.sendAck(ackRequest);


    }

    public static void sendTryingResponse(ServerTransaction st) throws SipException {
        try {
            Response response = SipUtilities.createResponse(st, Response.TRYING);
            st.sendResponse(response);
        } catch (InvalidArgumentException ex) {
            logger.error("Unexpected exception",ex);
            throw new SipXbridgeException("Unexpected ",ex);
        }
    }
    
	

}
