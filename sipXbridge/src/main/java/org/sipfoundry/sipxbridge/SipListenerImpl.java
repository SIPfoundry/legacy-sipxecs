/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.header.Via;
import gov.nist.javax.sip.stack.SIPClientTransaction;

import java.text.ParseException;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;

import javax.sdp.MediaDescription;
import javax.sdp.SdpFactory;
import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.InvalidArgumentException;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.TransactionUnavailableException;
import javax.sip.address.SipURI;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import junit.framework.TestCase;

import org.apache.log4j.Logger;

/**
 * This is the listener that fields all request and response events from the
 * stack. INVITES, ACKS, BYES get handled by the BackToBackUserAgentManager.
 * 
 * @author M. Ranganathan
 * 
 */
public class SipListenerImpl implements SipListener {

    private static Logger logger = Logger.getLogger(SipListenerImpl.class);

    public void processDialogTerminated(DialogTerminatedEvent dte) {

        CallIdHeader cid = dte.getDialog().getCallId();
        BackToBackUserAgent b2bua = Gateway.getCallControlManager()
                .getBackToBackUserAgent(dte.getDialog());
        if (b2bua != null) {
            b2bua.removeDialog(dte.getDialog());

        }

    }

    public void processIOException(IOExceptionEvent arg0) {
        // TODO Auto-generated method stub

    }

    /*
     * (non-Javadoc)
     * 
     * @see javax.sip.SipListener#processRequest(javax.sip.RequestEvent)
     */
    public void processRequest(RequestEvent requestEvent) {

        logger.debug("Gateway: got an invoming request "
                + requestEvent.getRequest());

        Request request = requestEvent.getRequest();
        String method = request.getMethod();
        SipProvider provider = (SipProvider) requestEvent.getSource();

        if (method.equals(Request.INVITE) || method.equals(Request.ACK)
                || method.equals(Request.CANCEL) || method.equals(Request.BYE)
                || method.equals(Request.OPTIONS)
                || method.equals(Request.REFER))
            Gateway.getCallControlManager().processRequest(requestEvent);

        else {
            try {
                Response response = ProtocolObjects.messageFactory
                        .createResponse(Response.METHOD_NOT_ALLOWED, request);
                ServerTransaction st = requestEvent.getServerTransaction();
                if (st == null) {
                    st = provider.getNewServerTransaction(request);

                }
                st.sendResponse(response);
            } catch (TransactionAlreadyExistsException ex) {
                logger.error("transaction already exists", ex);
            } catch (SipException ex) {
                throw new RuntimeException("Unexpected exceptione", ex);
            } catch (ParseException ex) {
                logger.error("Unexpected exception ", ex);
                throw new RuntimeException("unexpected exception", ex);
            } catch (InvalidArgumentException ex) {
                logger.error("Unexpected exception ", ex);
                throw new RuntimeException("unexpected exception", ex);

            }
        }

    }

    public void processResponse(ResponseEvent responseEvent) {
        Response response = responseEvent.getResponse();
        CSeqHeader cseqHeader = (CSeqHeader) response
                .getHeader(CSeqHeader.NAME);

        String method = cseqHeader.getMethod();

        try {

            if (response.getStatusCode() == Response.PROXY_AUTHENTICATION_REQUIRED
                    || response.getStatusCode() == Response.UNAUTHORIZED) {

                assert responseEvent.getClientTransaction()
                        .getApplicationData() != null;
                ItspAccountInfo accountInfo = ((TransactionApplicationData) responseEvent
                        .getClientTransaction().getApplicationData()).itspAccountInfo;
                SipProvider provider = (SipProvider) responseEvent.getSource();
                Dialog dialog = responseEvent.getDialog();
                if (logger.isDebugEnabled()) {
                    logger.debug("SipListenerImpl : dialog = " + dialog);
                }
                BackToBackUserAgent b2bua = Gateway.getCallControlManager()
                        .getBackToBackUserAgent(responseEvent.getDialog());
                if (b2bua != null)
                    b2bua.removeDialog(dialog);

                ClientTransaction newClientTransaction = Gateway
                        .getSipSecurityManager().handleChallenge(response,
                                responseEvent.getClientTransaction(), provider);

                // Handle authenitcation responses locally.

                TransactionApplicationData tad = (TransactionApplicationData) responseEvent
                        .getClientTransaction().getApplicationData();
                tad.clientTransaction = newClientTransaction;
                newClientTransaction.setApplicationData(tad);
                if (b2bua != null) {
                    b2bua.addDialog(newClientTransaction.getDialog());
                    DialogApplicationData dialogApplicationData = (DialogApplicationData) dialog
                            .getApplicationData();

                    DialogApplicationData newDialogApplicationData = new DialogApplicationData(
                            b2bua);
                    newDialogApplicationData.peerDialog = dialogApplicationData.peerDialog;
                    newClientTransaction.getDialog().setApplicationData(
                            newDialogApplicationData);
                    /*
                     * Hook the application data pointer of the previous guy in
                     * the chain at us.
                     */
                    DialogApplicationData peerDialogApplicationData = (DialogApplicationData) dialogApplicationData.peerDialog
                            .getApplicationData();
                    peerDialogApplicationData.peerDialog = newClientTransaction
                            .getDialog();

                    if (logger.isDebugEnabled()) {
                        logger.debug("SipListenerImpl: New Dialog = "
                                + newClientTransaction.getDialog());
                    }

                }

                if (dialog != null
                        && dialog.getState() == DialogState.CONFIRMED) {
                    ToHeader toHeader = (ToHeader) newClientTransaction
                            .getRequest().getHeader(ToHeader.NAME);
                    if (toHeader.getTag() != null) {
                        dialog.sendRequest(newClientTransaction);
                    }

                } else {
                    newClientTransaction.sendRequest();
                }

                return;
            } else if (method.equals(Request.REGISTER)) {
                Gateway.getRegistrationManager().processResponse(responseEvent);
            } else if (method.equals(Request.INVITE)
                    || method.equals(Request.CANCEL)
                    || method.equals(Request.BYE)) {
                Gateway.getCallControlManager().processResponse(responseEvent);
            } else {
                logger.debug("dropping response " + method);
            }

        } catch (Exception ex) {
            ex.printStackTrace();
            TestCase.fail("Unexpected exception processing challenge");
        }

    }

    /**
     * Remove state. Drop B2Bua structrue from our table so we will drop all
     * requests corresponding to this call in future.
     */

    public void processTimeout(TimeoutEvent timeoutEvent) {
        // TODO - tear down both sides of the associated dialog.

    }

    public void processTransactionTerminated(TransactionTerminatedEvent arg0) {

    }

}
