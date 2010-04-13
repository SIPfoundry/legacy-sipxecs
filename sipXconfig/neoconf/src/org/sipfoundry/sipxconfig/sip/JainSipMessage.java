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

import javax.sip.ClientTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.message.Request;

public abstract class JainSipMessage extends AbstractMessage {
    private ClientTransaction m_clientTransaction;
    private SipStackBean m_helper;
    private byte[] m_payload;
    private String m_contentType;


    public JainSipMessage(SipStackBean helper, String contentType, byte[] payload) {
        m_helper = helper;
        m_payload = payload;
        m_contentType = contentType;
    }

    public JainSipMessage(SipStackBean helper) {
        this(helper, null, null);
    }

    protected Request createRequest(String requestType, String userName, String fromDisplayName,
            String fromAddrSpec, String toAddrSpec, boolean forwardingAllowed) {
        try {
            Request request = m_helper.createRequest(requestType, userName, fromDisplayName,
                    fromAddrSpec, toAddrSpec, forwardingAllowed);
            m_helper.addContent(request, m_contentType, m_payload);
            return request;

        } catch (ParseException e) {
            throw new SipxSipException(e);
        }
    }

    protected SipProvider getSipProvider() {
        return m_helper.getSipProvider();
    }

    protected SipStackBean getHelper() {
        return m_helper;
    }

    protected void setClientTransaction(ClientTransaction clientTransaction) {
        m_clientTransaction = clientTransaction;
    }

    public void send() {
        try {
            m_clientTransaction.sendRequest();
        } catch (SipException e) {
            throw new SipxSipException(e);
        }
    }
}
