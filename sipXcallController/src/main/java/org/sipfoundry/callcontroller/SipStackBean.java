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
import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.clientauthutils.AuthenticationHelper;
import gov.nist.javax.sip.clientauthutils.SecureAccountManager;
import gov.nist.javax.sip.header.HeaderFactoryImpl;
import gov.nist.javax.sip.header.extensions.ReferredByHeader;

import java.text.ParseException;
import java.util.Collections;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;
import java.util.Random;
import java.util.Timer;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.InvalidArgumentException;
import javax.sip.SipException;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.SipStack;
import javax.sip.address.Address;
import javax.sip.address.AddressFactory;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;
import javax.sip.header.AllowHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.EventHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.Header;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.ReferToHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Message;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Appender;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.jainsip.AbstactSipStackBean;
import org.sipfoundry.commons.siprouter.FindSipServer;

public class SipStackBean extends AbstactSipStackBean {

    private static final Logger logger = Logger.getLogger(SipStackBean.class);

    private static final String APPLICATION = "application";

    private static final String SDP = "sdp";
    
	private static SipStackBean instance;

    private int m_maxForwards = 70;
  
    private SipListenerImpl m_sipListener;

    private final Timer m_timer = new Timer();
    
    private HashMap<String,DialogContext> dialogContextTable = 
        new HashMap<String,DialogContext>();


    
   
    public static SipStackBean getInstance() throws Exception {
    	if (SipStackBean.instance == null ) {
    		SipStackBean.instance = new SipStackBean();
    		SipStackBean.instance.init();
    	}
    	return SipStackBean.instance;
    }
  
   
    
    public final SecureAccountManager getAccountManager() {
        return CallController.getAccountManager();
    }

    public final String getStackName() {
        return "callcontroller";
    }
   
    public final Appender getStackAppender () {
        return CallController.getAppender();
    }
    final String getSipDomain() {
    	 return  CallController.getCallControllerConfig().getSipxProxyDomain();
    }

    public final String getHostIpAddress() {
        return  CallController.getCallControllerConfig().getIpAddress();
    }

    private SipURI createOurSipUri(String userName) throws ParseException {
        return getAddressFactory().createSipURI(userName, getSipDomain());
    }

    private FromHeader createFromHeader(String fromDisplayName, SipURI fromAddress) throws ParseException {
        Address fromNameAddress = getAddressFactory().createAddress(fromDisplayName, fromAddress);
        return getHeaderFactory().createFromHeader(fromNameAddress, Integer.toString(Math.abs(new Random().nextInt())));
    }

    final ContactHeader createContactHeader() throws ParseException {
        return ((ListeningPointExt) getUdpListeningPoint()).createContactHeader();
    }

    final long getSequenceNumber(Message sipMessage) {
        CSeqHeader cseqHeader = (CSeqHeader) sipMessage.getHeader(CSeqHeader.NAME);
        return cseqHeader.getSeqNumber();
    }

    final ToHeader createToHeader(SipURI toURI) throws ParseException {
        Address toAddress = getAddressFactory().createAddress(toURI);   
        return getHeaderFactory().createToHeader(toAddress, null);
    }

    final ViaHeader createViaHeader() throws ParseException, InvalidArgumentException {

        String host = getUdpListeningPoint().getIPAddress();
        int port = getUdpListeningPoint().getPort();

        String transport = getUdpListeningPoint().getTransport();
        // Leave the via header branch Id null.
        // The transaction layer will assign the via header branch Id.
        return getHeaderFactory().createViaHeader(host, port, transport, null);
    }

    final Request createRequest(String requestType, String userName, String fromDisplayName,
            String fromAddrSpec, String addrSpec, boolean forwardingAllowed) throws ParseException {
       
        logger.debug(String.format("requestType = %s userName = %s fromDisplayName = %s " +
        		"fromAddrSpec = %s addSpec = %s , forwardingAllowed = %s",
        		requestType,userName,fromDisplayName,fromAddrSpec,addrSpec,forwardingAllowed));
        
        SipURI fromUri = (SipURI) getAddressFactory().createURI("sip:"+fromAddrSpec);
      
        try {
            FromHeader fromHeader = createFromHeader(fromDisplayName, fromUri);
            SipURI toURI = (SipURI)getAddressFactory().createURI("sip:"+addrSpec);
            ToHeader toHeader = createToHeader(toURI);
            SipURI requestURI = (SipURI) getAddressFactory().createURI("sip:"+addrSpec);
            // limit forwarding so will not route to voicemail.
            requestURI.setParameter("sipx-noroute", "Voicemail");
            // This is disabled for click to dial but enabled for conference.
            if (!forwardingAllowed) {
                requestURI.setParameter("sipx-userforward", "false");
            }
            // Dont play MOH if configured on bridge. This is relevant for INVITE to conference.
            // MOH will still play fine on the click to dial case if so configured. This is only
            // for
            // Preventing MOH playing when we transfer to conference bridge.
            requestURI.setParameter("sipxbridge-moh", "false");
            MaxForwardsHeader maxForwards = getHeaderFactory().createMaxForwardsHeader(m_maxForwards);
            ViaHeader viaHeader = createViaHeader();
            ContactHeader contactHeader = createContactHeader();
            CallIdHeader callIdHeader = getSipProvider().getNewCallId();

            CSeqHeader cSeqHeader = getHeaderFactory().createCSeqHeader(1L, requestType);
            Request request = getMessageFactory().createRequest(requestURI, requestType, callIdHeader, cSeqHeader,
                    fromHeader, toHeader, Collections.singletonList(viaHeader), maxForwards);

            // Set loose routing to the target.
            Hop hop =  new FindSipServer(logger).findServer(requestURI);
            SipURI sipUri = getAddressFactory().createSipURI(null, hop.getHost());
            sipUri.setPort(hop.getPort());
            sipUri.setLrParam();
            Address address = getAddressFactory().createAddress(sipUri);

            RouteHeader routeHeader = getHeaderFactory().createRouteHeader(address);
            request.setHeader(routeHeader);
            request.addHeader(contactHeader);
            return request;
        } catch (InvalidArgumentException e) {
            throw new SipxSipException(e);
        }
    }

    final void addContent(Request request, String contentType, byte[] payload) throws ParseException {
        if (contentType == null) {
            return;
        }
        String[] ct = contentType.split("/", 2);
        ContentTypeHeader contentTypeHeader = getHeaderFactory().createContentTypeHeader(ct[0], ct[1]);
        if (contentTypeHeader != null) {
            request.setContent(payload, contentTypeHeader);
        }
    }

    final void addEventHeader(Request request, String eventType) throws ParseException {
        EventHeader eventHeader = getHeaderFactory().createEventHeader(eventType);
        request.addHeader(eventHeader);
    }

    final void addHeader(Request request, String name, String value) throws ParseException {
        Header header = getHeaderFactory().createHeader(name, value);
        request.addHeader(header);
    }

    final void setContent(Message message, SessionDescription sessionDescription) {
        try {
            ContentTypeHeader cth = getHeaderFactory().createContentTypeHeader(APPLICATION, SDP);
            String sd = sessionDescription.toString();
            message.setContent(sd, cth);
        } catch (ParseException ex) {
            throw new SipxSipException(ex);
        }

    }

    final Response createResponse(Request request, int responseCode) throws ParseException {
        return getMessageFactory().createResponse(responseCode, request);
    }

    final String getCSeqMethod(Response response) {
        return ((CSeqHeader) response.getHeader(CSeqHeader.NAME)).getMethod();
    }

    final ReferToHeader createReferToHeader(String referToAddrSpec) {
        try {
            String referToUri = "sip:" + referToAddrSpec;
            SipURI sipUri = (SipURI) getAddressFactory().createURI(referToUri);
            Address address = getAddressFactory().createAddress(sipUri);
            ReferToHeader referToHeader = getHeaderFactory().createReferToHeader(address);
            return referToHeader;
        } catch (ParseException ex) {
            throw new SipxSipException(ex);
        }
    }

    final void tearDownDialog(Dialog dialog) {
        logger.debug("Tearinging Down Dialog : " + dialog);
        if (dialog == null) {
            return;
        }
        try {
            if (dialog.getState() == DialogState.CONFIRMED) {
                Request request = dialog.createRequest(Request.BYE);
                SipProvider provider = ((DialogExt) dialog).getSipProvider();
                ClientTransaction ctx = provider.getNewClientTransaction(request);
                dialog.sendRequest(ctx);
            } else if (dialog.getState() != DialogState.TERMINATED) {
                dialog.delete();
            }
        } catch (SipException e) {
            logger.error("Unexpected exception sending BYE", e);
        }
    }

    public ContentTypeHeader createContentTypeHeader() throws ParseException {
        return getHeaderFactory().createContentTypeHeader(APPLICATION, SDP);
    }

    public ReferredByHeader createReferredByHeader(String addrSpec) throws ParseException {
        String referredByUri = "sip:" + addrSpec;
        Address address = getAddressFactory().createAddress(getAddressFactory().createURI(referredByUri));
        return ((HeaderFactoryImpl) getHeaderFactory()).createReferredByHeader(address);
    }

    public ClientTransaction handleChallenge(Response response, ClientTransaction clientTransaction) {
        if (getAuthenticationHelper() == null) {
            return null;
        }
        try {
            return getAuthenticationHelper().handleChallenge(response, clientTransaction, getSipProvider(), 5);
        } catch (SipException e) {
            throw new SipxSipException(e);
        }
    }

    public String formatWithIpAddress(String format) {
        String ipAddress = getUdpListeningPoint().getIPAddress();
        return String.format(format, ipAddress);
    }


    /**
     * We set up a timer to terminate the INVITE dialog if we do not see a 200 OK in the transfer.
     *
     * @param dialog dialog to terminate
     */
    public void scheduleTerminate(Dialog dialog) {
        ReferTimerTask referTimerTask = new ReferTimerTask(dialog);
        m_timer.schedule(referTimerTask, 180000);
    }

    public void attachAllowHeader(Request request, List<String> methods) {
        try {
            for (String method: methods) {
                AllowHeader allowHeader = getHeaderFactory().createAllowHeader(method);
                request.addHeader(allowHeader);
            }
        } catch (Exception ex) {
            logger.error("Could not attach allow header", ex);
            throw new SipxSipException ("could not attach allow headers", ex);
        }
    }
    
    public String getSipDomain(String uri) throws Exception {
     	SipURI sipUri = (SipURI) getAddressFactory().createURI(uri);
		return sipUri.getHost();   	
    }

   
    @Override
    public String getLogLevel() {      
        return CallController.getCallControllerConfig().getLogLevel();
    }

    @Override
    public SipListener getSipListener(AbstactSipStackBean abstactSipStackBean) {
        if ( m_sipListener == null ) {
            m_sipListener = new SipListenerImpl(this);
        }
       return this.m_sipListener;
    }
    

    @Override
    public int getSipPort() {
       return CallController.getCallControllerConfig().getSipPort();
    }
    
    public SipURI getProxySipURI() {
        try {
            AddressFactory addressFactory = getAddressFactory();

            return addressFactory.createSipURI(null,
                    CallController.getCallControllerConfig().getSipxProxyDomain());
        } catch (Exception ex) {
            throw new CallControllerException(ex);
        }
    }
    
    public synchronized DialogContext createDialogContext(String key) {
        logger.debug("createDialogCOntext " + key);
        if (dialogContextTable.get(key) == null) {
            DialogContext dialogContext = new DialogContext();
            dialogContext.setKey(key);
            this.dialogContextTable.put(key, dialogContext);
        }
        return dialogContextTable.get(key);
    }
    
    public synchronized void removeDialogContext(String key ) {
        this.dialogContextTable.remove(key);
    }
    
    public synchronized DialogContext getDialogContext(String key ) {
        return this.dialogContextTable.get(key);
    }

   
}
