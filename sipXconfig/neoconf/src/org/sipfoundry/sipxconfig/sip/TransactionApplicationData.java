/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.sip;

import java.util.concurrent.SynchronousQueue;

import gov.nist.javax.sip.header.extensions.ReferredByHeader;

import java.util.EventObject;
import java.util.concurrent.TimeUnit;

import java.text.ParseException;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.InvalidArgumentException;
import javax.sip.ResponseEvent;
import javax.sip.SipException;
import javax.sip.TimeoutEvent;
import javax.sip.header.AllowHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ReferToHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Transaction context data. Register one of these per transaction to track data that is specific
 * to a transaction.
 * 
 */
class TransactionApplicationData {

    private static final Log LOG = LogFactory.getLog(TransactionApplicationData.class);

    private Operator m_operator;

    private SynchronousQueue<EventObject> m_queue = new SynchronousQueue<EventObject>();

    private JainSipMessage m_message;

    private SipStackBean m_helper;

    private int m_counter;

    public TransactionApplicationData(Operator operator, SipStackBean stackBean,
            JainSipMessage message) {
        m_operator = operator;
        m_message = message;
        m_helper = stackBean;
    }

    /**
     * Blocks until transaction completes or timeout elapses
     * 
     * @return false if timeout, true if response received
     * @throws InterruptedException
     */
    public EventObject block() throws InterruptedException {
        return m_queue.poll(5000, TimeUnit.MILLISECONDS);
    }

    public void response(ResponseEvent responseEvent) {
        try {
            Response response = responseEvent.getResponse();
            ClientTransaction clientTransaction = responseEvent.getClientTransaction();
            Dialog dialog = clientTransaction.getDialog();
            String method = m_helper.getCSeqMethod(response);
            LOG.debug("method = " + method);
            LOG.debug("Operator = " + this.m_operator);
            if (response.getStatusCode() == Response.PROXY_AUTHENTICATION_REQUIRED) {
                if (m_counter == 1) {
                    m_helper.tearDownDialog(dialog);
                    return;
                }
                ClientTransaction ctx = m_helper.getAuthenticationHelper().handleChallenge(
                        response, clientTransaction, m_helper.getSipProvider(), 5);
                m_counter++;
                ctx.setApplicationData(this);
                ctx.sendRequest();
            }

            if (m_operator == Operator.SEND_NOTIFY) {
                // We ignore 1xx responses. 2xx and above are put into the queue.
                if (responseEvent.getResponse().getStatusCode() / 100 >= 2) {
                    m_queue.add(responseEvent);
                }
            } else if (method.equals(Request.INVITE)) {
                if (response.getStatusCode() / 100 == 2) {
                    if (this.m_operator == Operator.SEND_3PCC_REFER_CALL_SETUP) {
                        long seqno = m_helper.getSequenceNumber(response);
                        Request ack = dialog.createAck(seqno);
                        dialog.sendAck(ack);
                        InviteMessage inviteMessage = (InviteMessage) this.m_message;

                        // Create a REFER pointing back at the caller.
                        String fromAddrSpec = inviteMessage.getFromAddrSpec();
                        Request referRequest = dialog.createRequest(Request.REFER);
                        referRequest.removeHeader(AllowHeader.NAME);
                        ReferToHeader referTo = m_helper.createReferToHeader(fromAddrSpec);
                        referRequest.setHeader(referTo);
                        ReferredByHeader referredBy = m_helper
                                .createReferredByHeader(inviteMessage.getToAddrSpec());
                        referRequest.setHeader(referredBy);
                        ContactHeader contactHeader = m_helper.createContactHeader();
                        referRequest.setHeader(contactHeader);
                        ClientTransaction ctx = m_helper.getSipProvider()
                                .getNewClientTransaction(referRequest);

                        // And send it to the other side.
                        TransactionApplicationData tad = new TransactionApplicationData(
                                Operator.SEND_REFER, m_helper, null);
                        ctx.setApplicationData(tad);

                        dialog.sendRequest(ctx);

                    }
                }
            } else if (method.equals(Request.REFER)) {
                LOG.debug("Got REFER Response " + response.getStatusCode());
                // We set up a timer to terminate the INVITE dialog if we do not see a 200 OK
                // in the transfer.
                m_helper.getTimer().schedule(new ReferTimerTask(dialog), 180000);
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

    /**
     * @return the m_operator
     */
    public Operator getOperator() {
        return m_operator;
    }

}
