/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.callcontroller;

import gov.nist.javax.sip.TransactionExt;
import gov.nist.javax.sip.clientauthutils.UserCredentialHash;
import gov.nist.javax.sip.header.extensions.ReferredByHeader;

import java.text.ParseException;
import java.util.EventObject;
import java.util.Iterator;
import java.util.TimerTask;
import java.util.concurrent.SynchronousQueue;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.InvalidArgumentException;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionState;
import javax.sip.header.AllowHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.Header;
import javax.sip.header.ReferToHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxrest.RestServer;
import org.sipfoundry.sipxrest.SipHelper;

/**
 * Transaction context data. Register one of these per transaction to track data that is specific
 * to a transaction.
 * 
 */
class TransactionContext {

    private static final Logger LOG = Logger.getLogger(TransactionContext.class);

    private final Operator m_operator;

    private final SynchronousQueue<EventObject> m_queue = new SynchronousQueue<EventObject>();

    private final SipHelper m_helper;

    private int m_counter = -1;

    private UserCredentialHash m_userCredentials;

    private int m_timeout = -1;

    private ClientTransaction m_clientTransaction;

    private ServerTransaction m_serverTransaction;

    private InviteMessage m_sipMessage;

    private String target;

    public TransactionContext(Operator operator, int timeout, ClientTransaction clientTransaction) {
        m_operator = operator;
        m_timeout = timeout;
        m_clientTransaction = clientTransaction;
        m_helper = SipListenerImpl.getInstance().getHelper();

        Request request = clientTransaction.getRequest();

        if (request.getMethod().equals(Request.INVITE)) {

            RestServer.timer.schedule(new TimerTask() {

                @Override
                public void run() {
                    try {
                        if (m_clientTransaction.getState() != TransactionState.TERMINATED) {
                            Request cancelRequest = m_clientTransaction.createCancel();
                            SipProvider provider = ((TransactionExt) m_clientTransaction)
                                    .getSipProvider();
                            ClientTransaction ctx = provider
                                    .getNewClientTransaction(cancelRequest);
                            ctx.sendRequest();
                        }
                    } catch (Exception ex) {
                        LOG.debug("Exception in canceling request", ex);
                    }

                }

            }, m_timeout * 1000);
        }
    }

    public TransactionContext(Operator operator, ClientTransaction clientTransaction) {
        this.m_clientTransaction = clientTransaction;
        this.m_operator = operator;
        this.m_helper = SipListenerImpl.getInstance().getHelper();
        clientTransaction.setApplicationData(this);
        this.m_timeout = 30;
    }

    public void response(ResponseEvent responseEvent) {
        try {
            Response response = responseEvent.getResponse();
            ClientTransaction clientTransaction = responseEvent.getClientTransaction();
            Dialog dialog = responseEvent.getDialog();
            String method = SipHelper.getCSeqMethod(response);
            DialogContext dialogContext = (DialogContext) clientTransaction.getDialog()
                    .getApplicationData();
            
            SipHelper sipHelper = SipListenerImpl.getInstance().getHelper();

            LOG.debug("method = " + method);
            LOG.debug("Operator = " + m_operator);
            LOG.debug("dialog = " + dialog);
            if (response.getStatusCode() == Response.PROXY_AUTHENTICATION_REQUIRED) {
                if (m_counter == 1) {
                    m_helper.tearDownDialog(dialog);
                    return;
                }
                ClientTransaction ctx = m_helper.handleChallenge(response, clientTransaction);
                /* The old dialog is terminated -- take him out */
                dialogContext.removeMe(dialog);
                dialogContext.addDialog(ctx.getDialog(), ctx.getRequest());
                ctx.getDialog().setApplicationData(dialogContext);
                m_counter++;
                if (ctx != null) {
                    ctx.setApplicationData(this);
                    this.m_clientTransaction = ctx;
                    if (ctx.getDialog().getState() == DialogState.CONFIRMED) {
                        ctx.getDialog().sendRequest(ctx);
                    } else {
                        ctx.sendRequest();
                    }
                }
            } else if (m_operator == Operator.SEND_NOTIFY) {
                // We ignore 1xx responses. 2xx and above are put into the queue.
                if (responseEvent.getResponse().getStatusCode() / 100 >= 2) {
                    m_queue.add(responseEvent);
                }
            } else if (method.equals(Request.INVITE)) {
                if (response.getStatusCode() / 100 == 2) {
                    if (m_operator == Operator.SEND_3PCC_REFER_CALL_SETUP) {
                        long seqno = SipHelper.getSequenceNumber(response);
                        Request ack = dialog.createAck(seqno);
                        dialog.sendAck(ack);
                        InviteMessage inviteMessage = (InviteMessage) m_sipMessage;

                        // Create a REFER pointing back at the caller.
                        Request referRequest = dialog.createRequest(Request.REFER);
                        referRequest.removeHeader(AllowHeader.NAME);
                        String referTarget = inviteMessage.getCalledPartyAddrSpec();
                        ReferToHeader referTo = m_helper.createReferToHeader(referTarget);
                        referRequest.setHeader(referTo);
                        ReferredByHeader referredBy = m_helper
                                .createReferredByHeader(inviteMessage.getCallingPartyAddrSpec());
                        referRequest.setHeader(referredBy);
                        ContactHeader contactHeader = m_helper.createContactHeader();
                        referRequest.setHeader(contactHeader);
                        ClientTransaction ctx = m_helper.getSipProvider()
                                .getNewClientTransaction(referRequest);

                        // And send it to the other side.
                        TransactionContext tad = new TransactionContext(Operator.SEND_REFER, ctx);
                        tad.setUserCredentials(m_userCredentials);
                        ctx.setApplicationData(tad);
                        dialog.sendRequest(ctx);
                    } else if (this.m_operator == Operator.SEND_3PCC_CALL_SETUP1) {
                        long seqno = SipHelper.getSequenceNumber(response);
                        Request ack = dialog.createAck(seqno);
                        String sdpAnswer = new String(response.getRawContent());    
                        DialogContext.get(dialog).setLastSdpReceived(dialog, sdpAnswer);
                        dialog.sendAck(ack);
                        InviteMessage inviteMessage = (InviteMessage) this.m_sipMessage;

                        // Create a new INVITE ( no sdp ).
                        String toAddrSpec = inviteMessage.getAgentAddrSpec();
                        String fromAddrSpec = inviteMessage.getCalledPartyAddrSpec();
                        String agentAddrSpec = inviteMessage.getAgentAddrSpec();
                        InviteMessage newMessage = new InviteMessage(this.m_userCredentials,
                                null, agentAddrSpec, fromAddrSpec, toAddrSpec, this.m_timeout);

                        // And send it to the other side
                        newMessage.createAndSend(DialogContext.get(dialog), method,
                                Operator.SEND_3PCC_CALL_SETUP2, null);

                    } else if (this.m_operator == Operator.SEND_3PCC_CALL_SETUP2) {
                        // Extract the SDP of the offer and increment the version
                        if (response.getContentLength().getContentLength() == 0) {
                            LOG.error("Response contains no SDP body");
                            dialogContext.tearDownDialogs("Expecting SDP Body");
                            return;
                        }
                        // Extract the offer 2
                        SessionDescription newSd = SipHelper
                                .incrementSessionDescriptionVersionNumber(response);
                        DialogContext dat = (DialogContext) dialog.getApplicationData();
                        SessionDescription sd = SipHelper.getSessionDescription(response);
                        dat.setLastResponse(response);
                        dat.setLastSdpReceived(dialog,sd.toString() );
                        Dialog peerDialog = dat.getPeer(dialog);
                        if (peerDialog != null) {
                            InviteMessage newMessage = new InviteMessage(this.m_userCredentials,
                                    peerDialog);
                            Request request = newMessage.getRequest();
                            m_helper.setContent(request, newSd);
                            newMessage.createAndSend(peerDialog, Request.INVITE,
                                    Operator.SEND_3PCC_CALL_SETUP3, newSd.toString());
                        } else {
                            LOG.debug("Peer Dialog is NULL ");
                        }
                    } else if (m_operator == Operator.SEND_3PCC_CALL_SETUP3) {
                        DialogContext dat = (DialogContext) dialog.getApplicationData();
                        Dialog peerDialog = dat.getPeer(dialog);
                        long seqno = SipHelper.getSequenceNumber(dat.getLastResponse());
                        Request ack = peerDialog.createAck(seqno);
                        SessionDescription newSd = SipHelper
                                .decrementSessionDescriptionVersionNumber(response);
                        m_helper.setContent(ack, newSd);
                        DialogContext.get(peerDialog).setLastSdpSent(peerDialog, newSd.toString());      
                        peerDialog.sendAck(ack);
                        seqno = SipHelper.getSequenceNumber(response);
                        ack = dialog.createAck(seqno);
                        dialog.sendAck(ack);                 
                    } else if (m_operator == Operator.FORWARD_REQUEST) {
                        DialogContext dat = (DialogContext) dialog.getApplicationData();
                        dat.setLastResponse(response);
                        dat.setPendingOperation(PendingOperation.PENDING_ACK);
                        this.forwardResponse(responseEvent);
                    } else if (m_operator == Operator.SOLICIT_SDP_OFFER) {
                        if ( response.getRawContent() == null ) {
                            dialogContext.tearDownDialogs("Protocol Error - null SDP Body");
                            return;
                        }
                        dialogContext.setLastResponse(response);
                        dialogContext.sendLastSdpSentInAck(dialog,response);
                        String sdp = new String(response.getRawContent());
                        
                        dialogContext.sendSdpOffer(target, sdp);
                    } else if (m_operator == Operator.SEND_SDP_OFFER) {
                        long ackSeqno = SipHelper.getSequenceNumber(response);
                        Request ack = dialog.createAck(ackSeqno);
                        dialog.sendAck(ack);
                        Dialog peer = DialogContext.get(dialog).getPeer(dialog);
                        if ( peer == null || peer.getState() == DialogState.TERMINATED) {
                            DialogContext.get(dialog).tearDownDialogs("Peer dialog terminated");
                        }
                       
                       if (dialogContext.getPendingOperation() == PendingOperation.PENDING_RE_INVITE) {
                          dialogContext.reSendSdpOffereToPeerDialog(dialog, response, PendingOperation.NONE);
                       }
                       
                    } else if (m_operator == Operator.RESEND_SDP_OFFER_TO_PEER) {
                        long ackSeqno = SipHelper.getSequenceNumber(response);
                        Request ack = dialog.createAck(ackSeqno);
                        dialog.sendAck(ack);
                        if (dialogContext.getPendingOperation() == PendingOperation.PENDING_RE_INVITE) {
                             dialogContext.reSendSdpOffereToPeerDialog(dialog, response, PendingOperation.NONE);
                        }                
                    } else
                        throw new IllegalStateException("Unknown state " + m_operator);
                } else if (response.getStatusCode() / 100 > 2) {
                    dialogContext.removeMe(dialog);
                }
            } else if (m_operator == Operator.FORWARD_REQUEST) {
                this.forwardResponse(responseEvent);
            } else if (method.equals(Request.REFER)) {
                LOG.debug("Got REFER Response " + response.getStatusCode());
                // We set up a timer to terminate the INVITE dialog if we do not see a 200 OK in
                // the transfer.
                SipUtils.scheduleTerminate(dialog, 32);
            } 
        } catch (InvalidArgumentException e) {
            LOG.error("Invalid argument", e);
            throw new SipxSipException(e);
        } catch (SipException e) {
            LOG.error("SipException ", e);
            throw new SipxSipException(e);
        } catch (ParseException e) {
            LOG.error("ParseException ", e);
            throw new SipxSipException(e);
        } catch (SipxSipException e) {
            LOG.error("SipxSipException ", e);
            throw e;
        }
    }

    private void forwardResponse(ResponseEvent responseEvent) throws SipException,
            InvalidArgumentException {
        try {
            Response response = responseEvent.getResponse();
            Request request = m_serverTransaction.getRequest();
            Response newResponse = m_helper.createResponse(request, response.getStatusCode());
            byte[] content = response.getRawContent();
            Dialog peerDialog = m_serverTransaction.getDialog();
            if (content != null) {
                ContentTypeHeader cth = (ContentTypeHeader) response
                        .getHeader(ContentTypeHeader.NAME);
                newResponse.setContent(content, cth);
            }
            if (response.getHeader(ContactHeader.NAME) != null) {
                ContactHeader cth = m_helper.createContactHeader();
                newResponse.setHeader(cth);
            }
            for (Iterator it = response.getHeaderNames(); it.hasNext();) {
                String headerName = (String) it.next();
                if (newResponse.getHeader(headerName) == null) {
                    for (Iterator headerIterator = response.getHeaders(headerName); headerIterator
                            .hasNext();) {
                        Header header = (Header) headerIterator.next();
                        newResponse.addHeader((Header) header.clone());
                    }
                }
            }
            DialogContext.get(peerDialog).setLastResponse(newResponse);
            if (content != null) {
                DialogContext.get(peerDialog).setLastSdpReceived(peerDialog, new String(content));
            }

            m_serverTransaction.sendResponse(newResponse);
        } catch (ParseException ex) {
            throw new SipException("Unexpected parse exception ", ex);
        }

    }

    public void timeout(TimeoutEvent timeoutEvent) {
        LOG.debug("Timeout processing");
        if (m_operator == Operator.SEND_NOTIFY) {
            m_queue.add(timeoutEvent);
        } else {
            ClientTransaction ctx = timeoutEvent.getClientTransaction();
            Dialog dialog = ctx.getDialog();
            m_helper.tearDownDialog(dialog);
        }
    }

    public UserCredentialHash getUserCredentials() {
        return m_userCredentials;
    }

    public void setUserCredentials(UserCredentialHash userCredentials) {
        m_userCredentials = userCredentials;
    }

    public void setServerTransaction(ServerTransaction serverTransaction) {
        this.m_serverTransaction = serverTransaction;
    }

    public void setSipMessage(InviteMessage sipMessage) {
        this.m_sipMessage = sipMessage;
    }

    public static TransactionContext attach(ClientTransaction ctx, Operator operator) {
        TransactionContext txc = new TransactionContext(operator, ctx);
        return txc;
    }

    public void setTarget(String target) {
        this.target = target;
    }

}
