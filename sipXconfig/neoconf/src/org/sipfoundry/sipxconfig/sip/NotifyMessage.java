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

import java.text.ParseException;
import java.util.EventObject;

import javax.sip.ClientTransaction;
import javax.sip.ResponseEvent;
import javax.sip.SipException;
import javax.sip.TimeoutEvent;
import javax.sip.message.Request;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

class NotifyMessage extends JainSipMessage {
    static final Log LOG = LogFactory.getLog(NotifyMessage.class);

    private String m_addrSpec;
    private String m_eventType;

    public NotifyMessage(SipStackBean helper, String addrSpec, String eventType,
            String contentType, byte[] payload) {
        super(helper, contentType, payload);
        m_addrSpec = addrSpec;
        m_eventType = eventType;
    }

    public NotifyMessage(SipStackBean helper, String addrSpec, String eventType) {
        super(helper);
        m_addrSpec = addrSpec;
        m_eventType = eventType;
    }

    public void createAndSend() {
        try {
            Request request = createRequest(Request.NOTIFY, m_addrSpec);
            getHelper().addEventHeader(request, m_eventType);
            getHelper().addHeader(request, "Subscription-State", "active");
            ClientTransaction clientTx = getSipProvider().getNewClientTransaction(request);
            TransactionApplicationData tad = new TransactionApplicationData(Operator.SEND_NOTIFY);
            clientTx.setApplicationData(tad);
            clientTx.sendRequest();
            EventObject eventObject = tad.block();
            if (eventObject == null) {
                throw new SipException("Timed out waiting for response");
            } else if (eventObject instanceof TimeoutEvent) {
                throw new SipException("Timeout occured waiting for response");
            } else if (eventObject instanceof ResponseEvent) {
                ResponseEvent responseEvent = (ResponseEvent) eventObject;
                if (responseEvent.getResponse().getStatusCode() / 100 > 2) {
                    throw new SipException("Error returned by transaction "
                            + responseEvent.getResponse().getStatusCode() + " "
                            + responseEvent.getResponse().getReasonPhrase());
                }

            } else {
                LOG.warn("Unexpected event -- ignoring");
            }
        } catch (ParseException e) {
            throw new SipxSipException(e);
        } catch (SipException e) {
            throw new SipxSipException(e);
        } catch (InterruptedException e) {
            throw new SipxSipException(e);
        }
    }
}
