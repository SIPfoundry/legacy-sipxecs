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
import javax.sip.Dialog;
import javax.sip.SipException;
import javax.sip.message.Request;

public class InviteMessage extends JainSipMessage {

    private String m_toAddrSpec;

    private String m_fromAddrSpec;

    private Operator m_operator;

    private Request m_request;

    private Dialog m_dialog;

    /**
     * @param toAddressSpec the target for the 3pcc.
     */
    public InviteMessage(SipStackBean helper, String fromAddressSpec, String toAddressSpec, Operator operator) {
        super(helper, null, null);
        try {
            m_toAddrSpec = toAddressSpec;
            m_fromAddrSpec = fromAddressSpec;
            m_operator = operator;
            m_request = createRequest(Request.INVITE, m_fromAddrSpec, m_toAddrSpec);
            String sdpBody = helper.createSdpBody();
            m_request.setContent(sdpBody, helper.createContentTypeHeader());
        } catch (ParseException ex) {
            throw new SipxSipException(ex);
        }
    }

    public ClientTransaction createAndSend() {
        try {
            ClientTransaction ctx = getSipProvider().getNewClientTransaction(m_request);
            TransactionApplicationData tad = new TransactionApplicationData(m_operator, this.getHelper(), this);
            ctx.setApplicationData(tad);
            if (m_dialog == null) {
                ctx.sendRequest();
            } else {
                m_dialog.sendRequest(ctx);
            }
            return ctx;
        } catch (SipException ex) {
            throw new SipxSipException(ex);
        }

    }

    public ClientTransaction createTransaction() {
        try {
            ClientTransaction ctx = getSipProvider().getNewClientTransaction(m_request);
            TransactionApplicationData tad = new TransactionApplicationData(m_operator, this.getHelper(), this);
            ctx.setApplicationData(tad);
            setClientTransaction(ctx);
            return ctx;
        } catch (SipException ex) {
            throw new SipxSipException(ex);
        }

    }

    public String getToAddrSpec() {
        return m_toAddrSpec;
    }

    public String getFromAddrSpec() {
        return m_fromAddrSpec;
    }

    public Request getRequest() {
        return m_request;
    }
}
