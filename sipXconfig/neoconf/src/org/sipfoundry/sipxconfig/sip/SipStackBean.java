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
import java.util.Properties;
import java.util.TooManyListenersException;

import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.InvalidArgumentException;
import javax.sip.ListeningPoint;
import javax.sip.ObjectInUseException;
import javax.sip.PeerUnavailableException;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.SipFactory;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.SipStack;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.TransportNotSupportedException;
import javax.sip.address.Address;
import javax.sip.address.AddressFactory;
import javax.sip.address.SipURI;
import javax.sip.address.URI;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.EventHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.HeaderFactory;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.MessageFactory;
import javax.sip.message.Request;

import org.sipfoundry.sipxconfig.common.SipUri;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.CONFIG_SERVER;

/**
 * Spring adapter for JAIN SIP factories
 */
public class SipStackBean implements InitializingBean {
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
    
    private CallId m_id = new CallId();

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
            m_addressFactory = factory.createAddressFactory();
            m_headerFactory = factory.createHeaderFactory();
            m_messageFactory = factory.createMessageFactory();

            ListeningPoint listeningPoint = stack.createListeningPoint(m_hostIpAddress, m_port, m_transport);
            m_sipProvider = stack.createSipProvider(listeningPoint);
            DefaultSipListener sipListener = new DefaultSipListener();
            m_sipProvider.addSipListener(sipListener);
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

    public void setProperties(Properties properties) {
        m_properties = properties;
    }

    public SipProvider getSipProvider() {
        return m_sipProvider;
    }

    public void setPort(int port) {
        m_port = port;
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

    private SipURI createOurSipUri() throws ParseException {
        return m_addressFactory.createSipURI(CONFIG_SERVER.getUserName(), m_hostName);
    }

    public FromHeader createFromHeader() throws ParseException {
        SipURI fromAddress = createOurSipUri();
        Address fromNameAddress = m_addressFactory.createAddress(fromAddress);
        return m_headerFactory.createFromHeader(fromNameAddress, null);
    }

    public ContactHeader createContactHeader() throws ParseException {
        SipURI contactURI = createOurSipUri();
        contactURI.setPort(m_port);
        Address contactAddress = m_addressFactory.createAddress(contactURI);

        return m_headerFactory.createContactHeader(contactAddress);
    }

    final ToHeader createToHeader(String toAddrSpec) throws ParseException {
        Address toNameAddress = m_addressFactory.createAddress(toAddrSpec);
        return m_headerFactory.createToHeader(toNameAddress, null);
    }

    final ViaHeader createViaHeader() throws ParseException, InvalidArgumentException {
        String branchId = Long.toHexString(m_id.get());
        return m_headerFactory.createViaHeader(m_proxyHost, m_proxyPort, m_transport, branchId);
    }

    protected Request createRequest(String requestType, String addrSpec) throws ParseException {
        try {
            FromHeader fromHeader = createFromHeader();
            ToHeader toHeader = createToHeader(addrSpec);
            URI requestURI = m_addressFactory.createURI(addrSpec);
            MaxForwardsHeader maxForwards = m_headerFactory.createMaxForwardsHeader(m_maxForwards);
            ViaHeader viaHeader = createViaHeader();
            ContactHeader contactHeader = createContactHeader();
            CallIdHeader callIdHeader = m_sipProvider.getNewCallId();

            CSeqHeader cSeqHeader = m_headerFactory.createCSeqHeader(m_id.get(), requestType);
            Request request = m_messageFactory.createRequest(requestURI, requestType, callIdHeader, cSeqHeader,
                    fromHeader, toHeader, Collections.singletonList(viaHeader), maxForwards);

            request.addHeader(contactHeader);

            return request;

        } catch (InvalidArgumentException e) {
            throw new SipxSipException(e);
        }
    }

    public void addContent(Request request, String contentType, byte[] payload) throws ParseException {
        if (contentType == null) {
            return;
        }
        String[] ct = contentType.split("/", 2);
        ContentTypeHeader contentTypeHeader = m_headerFactory.createContentTypeHeader(ct[0], ct[1]);
        if (contentTypeHeader != null) {
            request.setContent(payload, contentTypeHeader);
        }
    }

    public void addEventHeader(Request request, String eventType) throws ParseException {
        EventHeader eventHeader = m_headerFactory.createEventHeader(eventType);
        request.addHeader(eventHeader);
    }

    static class DefaultSipListener implements SipListener {

        public void processDialogTerminated(DialogTerminatedEvent dialogTerminatedEvent) {
            // TODO Auto-generated method stub

        }

        public void processIOException(IOExceptionEvent exceptionEvent) {
            // TODO Auto-generated method stub

        }

        public void processRequest(RequestEvent requestEvent) {
            // TODO Auto-generated method stub

        }

        public void processResponse(ResponseEvent responseEvent) {
            // TODO Auto-generated method stub

        }

        public void processTimeout(TimeoutEvent timeoutEvent) {
            // TODO Auto-generated method stub

        }

        public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedEvent) {
            // TODO Auto-generated method stub

        }

    }
}
