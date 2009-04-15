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
import javax.sip.header.SubscriptionStateHeader;
import javax.sip.message.Request;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.CONFIG_SERVER;

class NotifyMessage extends JainSipMessage {
    private static final Log LOG = LogFactory.getLog(NotifyMessage.class);
    private static final String TIMEOUT_ERROR_MESSAGE = "Timed out waiting for response";
    private static final String TRANSACTION_ERROR_MESSAGE = "Error returned by transaction ";
    private static final String UNEXPECTED_EVENT = "Unexpected event -- ignoring";

    private final String m_addrSpec;
    private final String m_eventType;

    public NotifyMessage(SipStackBean helper, String addrSpec, String eventType, String contentType, byte[] payload) {
        super(helper, contentType, payload);
        m_addrSpec = addrSpec;
        m_eventType = eventType;
    }

    public NotifyMessage(SipStackBean helper, String addrSpec, String eventType) {
        super(helper);
        m_addrSpec = addrSpec;
        m_eventType = eventType;
    }

    @Override
    public ClientTransaction createAndSend() {
        try {
            Request request = createRequest(Request.NOTIFY, CONFIG_SERVER.getUserName(), null, null, m_addrSpec, false);
            getHelper().addEventHeader(request, m_eventType);
            getHelper().addHeader(request, SubscriptionStateHeader.NAME, SubscriptionStateHeader.ACTIVE);
            ClientTransaction clientTx = getSipProvider().getNewClientTransaction(request);
            TransactionApplicationData tad = new TransactionApplicationData(Operator.SEND_NOTIFY, this.getHelper(),
                    this);
            clientTx.setApplicationData(tad);
            clientTx.sendRequest();
            EventObject eventObject = tad.block();
            if (eventObject == null) {
                throw new SipException(TIMEOUT_ERROR_MESSAGE);
            } else if (eventObject instanceof TimeoutEvent) {
                throw new SipException(TIMEOUT_ERROR_MESSAGE);
            } else if (eventObject instanceof ResponseEvent) {
                ResponseEvent responseEvent = (ResponseEvent) eventObject;
                if (responseEvent.getResponse().getStatusCode() / 100 > 2) {
                    throw new SipException(TRANSACTION_ERROR_MESSAGE + responseEvent.getResponse().getStatusCode()
                            + responseEvent.getResponse().getReasonPhrase());
                }

            } else {
                LOG.warn(UNEXPECTED_EVENT);
            }
            return clientTx;
        } catch (ParseException e) {
            throw new SipxSipException(e);
        } catch (SipException e) {
            throw new SipxSipException(e);
        } catch (InterruptedException e) {
            throw new SipxSipException(e);
        }
    }
}
