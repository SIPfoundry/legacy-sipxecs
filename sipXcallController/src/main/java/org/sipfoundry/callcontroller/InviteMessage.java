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
import javax.sip.address.SipURI;
import javax.sip.header.AllowHeader;
import javax.sip.header.SubjectHeader;
import javax.sip.message.Request;

import org.sipfoundry.sipxrest.RestServer;
import org.sipfoundry.sipxrest.SipHelper;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;
import gov.nist.javax.sip.clientauthutils.UserCredentials;

public class InviteMessage extends JainSipMessage {

    // Dummy sdp body for initial INVITE
    private static final String SDP_BODY_FORMAT = "v=0\r\n" + "o=- 978416123 978416123 IN IP4 %1$s\r\n"
            + "s=SipXconfig\r\n" + "c=IN IP4 %1$s\r\n" + "t=0 0\r\n" + "m=audio 2222 RTP/AVP 0 101\r\n"
            + "a=sendrecv\r\n" + "a=rtpmap:0 PCMU/8000\r\n" + "a=rtpmap:101 telephone-event/8000\r\n";

    private static final List<String> METHODS = Arrays.asList(Request.INVITE, Request.ACK, Request.OPTIONS,
            Request.CANCEL, Request.BYE, Request.REFER, Request.NOTIFY);

    private final String m_toAddrSpec;

    private final String m_fromDisplayName;
    
    private final String m_fromAddrSpec;

    private final Operator m_operator;

    private Dialog m_dialog;

    private final UserCredentialHash m_userCredentials;

    private String m_referTarget;
    
    private boolean m_forwardingAllowed;

    private String subject;

    

    public InviteMessage(UserCredentialHash userCredentials, String fromDisplayName, 
            String fromAddressSpec, String toAddressSpec, String referTarget, 
            Operator operator) {
        super();
        m_toAddrSpec = toAddressSpec;
        m_fromDisplayName = fromDisplayName;
        m_fromAddrSpec = fromAddressSpec;
        m_referTarget = referTarget;
        m_operator = operator;
        m_userCredentials = userCredentials;
        
    }
    
    @Override
    public ClientTransaction createAndSend() {
        try {
           
            Request request = createRequest(Request.INVITE, m_userCredentials.getUserName(), m_fromDisplayName, 
                    m_fromAddrSpec, m_toAddrSpec, m_forwardingAllowed);
            if ( subject != null ) {
                SubjectHeader subjectHeader = 
                    RestServer.getSipStack().getHeaderFactory().createSubjectHeader(subject);
                request.addHeader(subjectHeader);
            }
            SipURI requestURI = (SipURI) request.getRequestURI();
            requestURI.setParameter("sipxbridge-moh", "false");
            
            SipHelper helper = SipListenerImpl.getInstance().getHelper();
            SipListenerImpl.getInstance().getHelper().attachAllowHeader(request,METHODS);
            String sdpBody = SipUtils.formatWithIpAddress(SDP_BODY_FORMAT);
            request.setContent(sdpBody, helper.createContentTypeHeader());
            ClientTransaction ctx = helper.getNewClientTransaction(request);
            TransactionApplicationData tad = new TransactionApplicationData(m_operator,  this);
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

    public String getToAddrSpec() {
        return m_toAddrSpec;
    }

    public String getFromAddrSpec() {
        return m_fromAddrSpec;
    }

    public String getReferTarget() {
        return m_referTarget;
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

}
