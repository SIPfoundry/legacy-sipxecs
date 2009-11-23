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

import java.text.ParseException;
import java.util.Arrays;
import java.util.List;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.address.SipURI;
import javax.sip.header.AllowHeader;
import javax.sip.header.SubjectHeader;
import javax.sip.message.Request;

import org.sipfoundry.sipxrest.RestServer;
import org.sipfoundry.sipxrest.SipHelper;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.clientauthutils.UserCredentialHash;
import gov.nist.javax.sip.clientauthutils.UserCredentials;

public class InviteMessage extends JainSipMessage {

    // Dummy sdp body for initial INVITE
    static final String SDP_BODY_FORMAT = "v=0\r\n" + "o=- %2$s 1 IN IP4 %1$s\r\n"
            + "s=SipXconfig\r\n" + "c=IN IP4 %1$s\r\n" + "t=0 0\r\n"
            + "m=audio 2222 RTP/AVP 0 101\r\n" + "a=sendrecv\r\n" + "a=rtpmap:0 PCMU/8000\r\n"
            + "a=rtpmap:101 telephone-event/8000\r\n";

    static final String SDP_BODY_NO_MEDIA_FORMAT = "v=0\r\n" + "o=- %2$s 1 IN IP4 %1$s\r\n"
            + "s=SipXconfig\r\n" + "c=IN IP4 %1$s\r\n" + "t=0 0\r\n" + "a=sendrecv\r\n";

    private static final List<String> METHODS = Arrays.asList(Request.INVITE, Request.ACK,
            Request.OPTIONS, Request.CANCEL, Request.BYE, Request.REFER, Request.NOTIFY);

    private final String m_callingPartyAddrSpec;

    private final String m_fromDisplayName;

    private final String m_agentAddrSpec;

    private Dialog m_dialog;

    private final UserCredentialHash m_userCredentials;

    private String m_calledPartyAddrSpec;

    private boolean m_forwardingAllowed;

    private String subject;

    private int m_timeout;

    private Request m_request;

    private Operator m_operator;

    public InviteMessage(UserCredentialHash userCredentials, String fromDisplayName,
            String agentAddressSpec, String callingPartyAddressSpec, String calledPartyAddrSpec,
            int timeout) {
        super();
        m_callingPartyAddrSpec = callingPartyAddressSpec;
        m_fromDisplayName = fromDisplayName;
        m_agentAddrSpec = agentAddressSpec;
        m_calledPartyAddrSpec = calledPartyAddrSpec;
        m_userCredentials = userCredentials;
        m_timeout = timeout;

    }

    /*
     * An In-dialog invite request
     */
    public InviteMessage(UserCredentialHash userCredentials, Dialog dialog) throws SipException {
        super();
        m_request = DialogContext.get(dialog).getRequest(dialog);
        m_agentAddrSpec = SipHelper.getFromAddrSpec(m_request);
        this.m_callingPartyAddrSpec = SipHelper.getFromAddrSpec(m_request);
        this.m_calledPartyAddrSpec = SipHelper.getToAddrSpec(m_request);
        this.m_userCredentials = userCredentials;
        this.m_fromDisplayName = SipHelper.getFromDisplayName(m_request);
    }

    public ClientTransaction createClientTransactionFromAgentToCalledParty() {
        try {
            m_request = createRequest(Request.INVITE, m_userCredentials.getUserName(),
                    m_fromDisplayName, m_agentAddrSpec, m_calledPartyAddrSpec,
                    m_forwardingAllowed);

            ClientTransaction ctx = getSipProvider().getNewClientTransaction(m_request);
            TransactionContext tad = new TransactionContext(m_operator,
                    m_timeout, ctx);
            ctx.setApplicationData(tad);
            setClientTransaction(ctx);
            return ctx;
        } catch (SipException ex) {
            throw new SipxSipException(ex);
        }

    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.callcontroller.JainSipMessage#createAndSend(org.sipfoundry.callcontroller.DialogContext,
     *      java.lang.String, org.sipfoundry.callcontroller.Operator, java.lang.String)
     */
    @Override
    public ClientTransaction createAndSend(DialogContext dialogContext, String method,
            Operator operator, String sdpBody) {
        try {

             m_request = createRequest(Request.INVITE, m_userCredentials.getUserName(),
                    m_fromDisplayName, m_agentAddrSpec, m_callingPartyAddrSpec,
                    m_forwardingAllowed);
            SipListenerImpl.getInstance().addCallId(SipHelper.getCallId(m_request));
            this.m_operator = operator;
            if (subject != null) {
                SubjectHeader subjectHeader = RestServer.getSipStack().getHeaderFactory()
                        .createSubjectHeader(subject);
                m_request.addHeader(subjectHeader);
            }
            SipURI requestURI = (SipURI) m_request.getRequestURI();
            requestURI.setParameter("sipxbridge-moh", "false");

            SipHelper helper = SipListenerImpl.getInstance().getHelper();
            SipListenerImpl.getInstance().getHelper().attachAllowHeader(m_request, METHODS);
            if (sdpBody != null) {
                m_request.setContent(sdpBody, helper.createContentTypeHeader());
            }
            ClientTransaction ctx = helper.getNewClientTransaction(m_request);
            Dialog dialog = ctx.getDialog();
            dialog.setApplicationData(dialogContext);
            dialogContext.addDialog(dialog, m_request);
            TransactionContext tad = new TransactionContext(operator, m_timeout,
                    ctx);
            tad.setSipMessage(this);
            tad.setUserCredentials(m_userCredentials);
            ctx.setApplicationData(tad);
            if (m_dialog == null) {
                ctx.sendRequest();
            } else {
                m_dialog.sendRequest(ctx);
            }
            return ctx;
        } catch (ParseException ex) {
            throw new SipxSipException(ex);
        } catch (Exception ex) {
            throw new SipxSipException(ex);
        }
    }

    public void createAndSend(Dialog peerDialog, String method, Operator operator, String sdp) throws SipException {
        Request request = peerDialog.createRequest(method);
        if (sdp != null) {
            SipListenerImpl.getInstance().getHelper().addSdpContent(request, sdp);
        }
        SipProvider provider = ((DialogExt) peerDialog).getSipProvider();
        ClientTransaction clientTransaction = provider.getNewClientTransaction(request);
        new TransactionContext(operator,clientTransaction);
        peerDialog.sendRequest(clientTransaction);
    }

    public String getCallingPartyAddrSpec() {
        return m_callingPartyAddrSpec;
    }

    public String getAgentAddrSpec() {
        return m_agentAddrSpec;
    }

    public String getCalledPartyAddrSpec() {
        return m_calledPartyAddrSpec;
    }

    public void setforwardingAllowed(boolean forwardingAllowed) {
        m_forwardingAllowed = forwardingAllowed;
    }

    public boolean isforwardingAllowed() {
        return m_forwardingAllowed;
    }

    public void setSubject(String subject) {
        this.subject = subject;
    }

    public Request getRequest() {
        return this.m_request;
    }

}
