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

import gov.nist.javax.sip.DialogExt;

import java.text.ParseException;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.message.Request;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxrest.SipHelper;

public abstract class JainSipMessage extends AbstractMessage {
    private static Logger LOG = Logger.getLogger(JainSipMessage.class);

    private ClientTransaction m_clientTransaction;

    private byte[] m_payload;
    private String m_contentType;
    private Dialog m_dialog;

    public JainSipMessage(String contentType, byte[] payload) {

        m_payload = payload;
        m_contentType = contentType;
    }

    protected JainSipMessage() {

    }

    public JainSipMessage(Dialog dialog) {
        this.m_dialog = dialog;
    }

    protected Request createRequest(String requestType, String userName, String fromDisplayName,
            String fromAddrSpec, String toAddrSpec, boolean forwardingAllowed) {
        try {
            Request request = getHelper().createRequest(requestType, userName, fromDisplayName,
                    fromAddrSpec, toAddrSpec, forwardingAllowed);
            getHelper().addContent(request, m_contentType, m_payload);
            return request;

        } catch (ParseException e) {
            throw new SipxSipException(e);
        }
    }

    protected SipProvider getSipProvider() {
        return getHelper().getSipProvider();
    }

    protected SipHelper getHelper() {
        return SipListenerImpl.getInstance().getHelper();
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

    @Override
    public ClientTransaction createAndSend(DialogContext dialogContext, String method,
            Operator operator, String body) {
        try {
            SipProvider provider = ((DialogExt) m_dialog).getSipProvider();
            Request request = m_dialog.createRequest(method);
            ClientTransaction ctx = provider.getNewClientTransaction(request);
            dialogContext.addDialog(ctx.getDialog(),request);
            TransactionContext tad = new TransactionContext(operator, ctx);
            m_dialog.sendRequest(ctx);
            return ctx;
        } catch (Exception ex) {
            LOG.error("Exception sending message");
            dialogContext.tearDownDialogs();
            return null;
        }

    }

}
