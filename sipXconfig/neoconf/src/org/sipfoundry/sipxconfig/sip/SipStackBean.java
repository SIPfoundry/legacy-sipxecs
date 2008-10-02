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
import java.util.Collections;
import java.util.List;
import java.util.Properties;
import java.util.Random;
import java.util.Timer;
import java.util.TooManyListenersException;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.InvalidArgumentException;
import javax.sip.ListeningPoint;
import javax.sip.ObjectInUseException;
import javax.sip.PeerUnavailableException;
import javax.sip.SipException;
import javax.sip.SipFactory;
import javax.sip.SipProvider;
import javax.sip.SipStack;
import javax.sip.TransportNotSupportedException;
import javax.sip.address.Address;
import javax.sip.address.AddressFactory;
import javax.sip.address.SipURI;
import javax.sip.address.URI;
import javax.sip.header.AllowHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.EventHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.Header;
import javax.sip.header.HeaderFactory;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.ReferToHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Message;
import javax.sip.message.MessageFactory;
import javax.sip.message.Request;
import javax.sip.message.Response;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.SipStackImpl;
import gov.nist.javax.sip.clientauthutils.AuthenticationHelper;
import gov.nist.javax.sip.header.HeaderFactoryImpl;
import gov.nist.javax.sip.header.extensions.ReferredByHeader;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.Appender;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.beans.factory.annotation.Required;

/**
 * Spring adapter for JAIN SIP factories
 */
public class SipStackBean implements InitializingBean {

    private static final Log LOG = LogFactory.getLog(SipStackBean.class);

    private static final String APPLICATION = "application";

    private static final String SDP = "sdp";

    private int m_port;

    private String m_hostName;

    private String m_hostIpAddress;

    private String m_transport = "udp";

    private int m_maxForwards = 70;

    private String m_proxyHost;

    private int m_proxyPort = SipUri.DEFAULT_SIP_PORT;

    private Properties m_properties;

    private AddressFactory m_addressFactory;

    private HeaderFactory m_headerFactory;

    private MessageFactory m_messageFactory;

    private SipProvider m_sipProvider;

    private ListeningPoint m_listeningPoint;

    private final CallId m_id = new CallId();

    private SipListenerImpl m_sipListener;

    private Appender m_logAppender;

    private CoreContext m_coreContext;

    private AuthenticationHelper m_authenticationHelper;

    private final Timer m_timer = new Timer();

    public void afterPropertiesSet() {
        SipFactory factory = SipFactory.getInstance();
        factory.setPathName("gov.nist");
        if (m_properties == null) {
            m_properties = new Properties();
        }
        // add more properties here if needed
        String errorMsg = "Cannot initialize SIP stack";
        try {
            SipStack stack = factory.createSipStack(m_properties);
            addLogAppender(stack);
            m_addressFactory = factory.createAddressFactory();
            m_headerFactory = factory.createHeaderFactory();
            m_messageFactory = factory.createMessageFactory();

            m_listeningPoint = stack.createListeningPoint(m_hostIpAddress, m_port, m_transport);
            m_sipProvider = stack.createSipProvider(m_listeningPoint);

            m_sipListener = new SipListenerImpl(this);
            m_sipProvider.addSipListener(m_sipListener);

            m_authenticationHelper = createAuthenticationHelper(stack);
        } catch (PeerUnavailableException e) {
            throw new BeanInitializationException(errorMsg, e);
        } catch (TransportNotSupportedException e) {
            throw new BeanInitializationException(errorMsg, e);
        } catch (InvalidArgumentException e) {
            throw new BeanInitializationException(errorMsg, e);
        } catch (ObjectInUseException e) {
            throw new BeanInitializationException(errorMsg, e);
        } catch (TooManyListenersException e) {
            throw new BeanInitializationException(errorMsg, e);
        }
    }

    private AuthenticationHelper createAuthenticationHelper(SipStack stack) {
        if (stack instanceof SipStackImpl) {
            SipStackImpl impl = (SipStackImpl) stack;
            AccountManagerImpl accountManager = new AccountManagerImpl();
            return impl.getAuthenticationHelper(accountManager, m_headerFactory);
        }
        return null;
    }

    private void addLogAppender(SipStack stack) {
        if (m_logAppender == null) {
            return;
        }
        // HACK: there should be a method in SipStack to change log appender
        if (stack instanceof SipStackImpl) {
            SipStackImpl impl = (SipStackImpl) stack;
            impl.addLogAppender(m_logAppender);
        }
    }

    public void setProperties(Properties properties) {
        m_properties = properties;
    }

    public SipProvider getSipProvider() {
        return m_sipProvider;
    }

    public void setPort(int port) {
        m_port = port;
    }

    public void setLogAppender(Appender logAppender) {
        m_logAppender = logAppender;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setHostName(String hostName) {
        m_hostName = hostName;
    }

    @Required
    public void setHostIpAddress(String hostIpAddress) {
        m_hostIpAddress = hostIpAddress;
    }

    public void setTransport(String transport) {
        m_transport = transport;
    }

    public void setMaxForwards(int maxForwards) {
        m_maxForwards = maxForwards;
    }

    @Required
    public void setProxyHost(String proxy) {
        m_proxyHost = proxy;
    }

    @Required
    public void setProxyPort(int port) {
        m_proxyPort = port;
    }

    final CoreContext getCoreContext() {
        return m_coreContext;
    }

    final String getHostName() {
        return m_hostName;
    }

    private SipURI createOurSipUri(String userName) throws ParseException {
        return m_addressFactory.createSipURI(userName, m_hostName);
    }

    private FromHeader createFromHeader(String fromDisplayName, SipURI fromAddress) throws ParseException {
        Address fromNameAddress = m_addressFactory.createAddress(fromDisplayName, fromAddress);
        return m_headerFactory.createFromHeader(fromNameAddress, Integer.toString(Math.abs(new Random().nextInt())));
    }

    final ContactHeader createContactHeader() throws ParseException {
        return ((ListeningPointExt) m_listeningPoint).createContactHeader();
    }

    final long getSequenceNumber(Message sipMessage) {
        CSeqHeader cseqHeader = (CSeqHeader) sipMessage.getHeader(CSeqHeader.NAME);
        return cseqHeader.getSeqNumber();
    }

    final ToHeader createToHeader(String toAddrSpec) throws ParseException {
        Address toNameAddress = m_addressFactory.createAddress(toAddrSpec);
        return m_headerFactory.createToHeader(toNameAddress, null);
    }

    final ViaHeader createViaHeader() throws ParseException, InvalidArgumentException {

        String host = m_listeningPoint.getIPAddress();
        int port = m_listeningPoint.getPort();

        String transport = m_listeningPoint.getTransport();
        // Leave the via header branch Id null.
        // The transaction layer will assign the via header branch Id.
        return m_headerFactory.createViaHeader(host, port, transport, null);
    }

    final Request createRequest(String requestType, String userName, String fromDisplayName,
            String fromAddrSpec, String addrSpec)
        throws ParseException {
        SipURI fromUri;
        if (fromAddrSpec == null) {
            fromUri = createOurSipUri(userName);
        } else {
            fromUri = (SipURI) m_addressFactory.createURI(fromAddrSpec);
        }
        try {
            FromHeader fromHeader = createFromHeader(fromDisplayName, fromUri);
            ToHeader toHeader = createToHeader(addrSpec);
            URI requestURI = m_addressFactory.createURI(addrSpec);
            MaxForwardsHeader maxForwards = m_headerFactory.createMaxForwardsHeader(m_maxForwards);
            ViaHeader viaHeader = createViaHeader();
            ContactHeader contactHeader = createContactHeader();
            CallIdHeader callIdHeader = m_sipProvider.getNewCallId();

            CSeqHeader cSeqHeader = m_headerFactory.createCSeqHeader(m_id.get(), requestType);
            Request request = m_messageFactory.createRequest(requestURI, requestType, callIdHeader, cSeqHeader,
                    fromHeader, toHeader, Collections.singletonList(viaHeader), maxForwards);

            // FIXME: we need to properly resolve address here
            SipURI sipUri = m_addressFactory.createSipURI(null, m_proxyHost);
            sipUri.setPort(m_proxyPort);
            sipUri.setLrParam();
            Address address = m_addressFactory.createAddress(sipUri);

            RouteHeader routeHeader = m_headerFactory.createRouteHeader(address);
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
        ContentTypeHeader contentTypeHeader = m_headerFactory.createContentTypeHeader(ct[0], ct[1]);
        if (contentTypeHeader != null) {
            request.setContent(payload, contentTypeHeader);
        }
    }

    final void addEventHeader(Request request, String eventType) throws ParseException {
        EventHeader eventHeader = m_headerFactory.createEventHeader(eventType);
        request.addHeader(eventHeader);
    }

    final void addHeader(Request request, String name, String value) throws ParseException {
        Header header = m_headerFactory.createHeader(name, value);
        request.addHeader(header);
    }

    final void setContent(Message message, SessionDescription sessionDescription) {
        try {
            ContentTypeHeader cth = m_headerFactory.createContentTypeHeader(APPLICATION, SDP);
            String sd = sessionDescription.toString();
            message.setContent(sd, cth);
        } catch (ParseException ex) {
            throw new SipxSipException(ex);
        }

    }

    final Response createResponse(Request request, int responseCode) throws ParseException {
        return m_messageFactory.createResponse(responseCode, request);
    }

    final String getCSeqMethod(Response response) {
        return ((CSeqHeader) response.getHeader(CSeqHeader.NAME)).getMethod();
    }

    final ReferToHeader createReferToHeader(String fromAddrSpec) {
        try {
            SipURI sipUri = (SipURI) m_addressFactory.createURI(fromAddrSpec);
            Address address = m_addressFactory.createAddress(sipUri);
            ReferToHeader referToHeader = m_headerFactory.createReferToHeader(address);
            return referToHeader;
        } catch (ParseException ex) {
            throw new SipxSipException(ex);
        }
    }

    final void tearDownDialog(Dialog dialog) {
        LOG.debug("Tearinging Down Dialog : " + dialog);
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
            LOG.error("Unexpected exception sending BYE", e);
        }
    }

    public ContentTypeHeader createContentTypeHeader() throws ParseException {
        return m_headerFactory.createContentTypeHeader(APPLICATION, SDP);
    }

    public ReferredByHeader createReferredByHeader(String addrSpec) throws ParseException {
        Address address = m_addressFactory.createAddress(m_addressFactory.createURI(addrSpec));
        return ((HeaderFactoryImpl) m_headerFactory).createReferredByHeader(address);
    }

    public ClientTransaction handleChallenge(Response response, ClientTransaction clientTransaction) {
        if (m_authenticationHelper == null) {
            return null;
        }
        try {
            return m_authenticationHelper.handleChallenge(response, clientTransaction, m_sipProvider, 5);
        } catch (SipException e) {
            throw new SipxSipException(e);
        }
    }

    public String formatWithIpAddress(String format) {
        String ipAddress = m_listeningPoint.getIPAddress();
        return String.format(format, ipAddress);
    }

    public AllowHeader createAllowHeader(List<String> methods) throws ParseException {
        return m_headerFactory.createAllowHeader(StringUtils.join(methods, ", "));
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
}
