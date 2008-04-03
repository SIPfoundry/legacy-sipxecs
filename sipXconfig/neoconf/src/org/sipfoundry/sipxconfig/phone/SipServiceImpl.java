/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Formatter;
import java.util.TimeZone;

import org.apache.commons.lang.CharEncoding;
import org.apache.commons.lang.math.RandomUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

/**
 * Implementation note: This is very, very basic sip implementation. Going forward, either use
 * sipXtack java binding or interact w/sipXpublisher thru some RPC to have it deliver sip messages
 * on behalf or sipXconfig
 */
public class SipServiceImpl implements SipService {

    private static final Log LOG = LogFactory.getLog(SipServiceImpl.class);

    /** Example: Tue, 15 Nov 1994 08:12:31 GMT */
    private SimpleDateFormat m_dateFormat;

    private String m_serverName;

    private String m_proxyHost;

    private int m_proxyPort = SipUri.DEFAULT_SIP_PORT;

    private DomainManager m_domainManager;

    public SipServiceImpl() {
        m_dateFormat = new SimpleDateFormat("EEE, d MMM yyyy kk:mm:ss z");
        m_dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
    }

    @Required
    public void setProxyHost(String proxy) {
        m_proxyHost = proxy;
    }

    @Required
    public void setProxyPort(int port) {
        m_proxyPort = port;
    }

    @Required
    public void setFromServerName(String serverName) {
        m_serverName = serverName;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void sendCheckSync(String uri) {
        sendNotify(uri, "Event: check-sync\r\n", new byte[0]);
    }

    public void sendNotify(String uri, String event, byte[] payload) {
        String domainName = m_domainManager.getDomain().getName();
        try {
            ByteArrayOutputStream msgStream = new ByteArrayOutputStream();
            Writer msgWriter = new OutputStreamWriter(msgStream, CharEncoding.US_ASCII);
            formatSipHeaders(msgWriter, uri, domainName, m_proxyPort, event, payload.length);
            msgWriter.flush();
            msgStream.write(payload);
            send(msgStream.toByteArray());
            msgStream.close();
        } catch (IOException e) {
            throw new RestartException("Could not send restart SIP message", e);
        }

    }

    void formatServerVia(Formatter f) {
        f.format("Via: SIP/2.0/UDP %s:%d;branch=%x\r\n", m_serverName, m_proxyPort, generateUniqueId());
    }

    String getServerUri() {
        return SipUri.format("sipuaconfig", m_serverName, m_proxyPort);
    }

    void send(byte[] sipBytes) throws IOException {
        LOG.info(new String(sipBytes, CharEncoding.US_ASCII));
        // not particular reason it's UDP other than we do not expect a response
        DatagramSocket socket = new DatagramSocket();
        InetAddress toAddress = InetAddress.getByName(m_proxyHost);
        DatagramPacket packet = new DatagramPacket(sipBytes, 0, sipBytes.length, toAddress, m_proxyPort);
        socket.send(packet);
        socket.disconnect();
    }

    void formatSipHeaders(Appendable header, String uri, String sipDomain, int port, String event, int payloadLen)
        throws IOException {
        String userId = SipUri.extractUser(uri);
        // The check-sync message is a flavor of unsolicited NOTIFY
        // this message does not require that the phone be enrolled
        // the message allows us to reboot a specific phone
        Formatter f = new Formatter(header);
        f.format("NOTIFY %s SIP/2.0\r\n", getNotifyRequestUri(userId, sipDomain, port));
        formatServerVia(f);
        f.format("From: %s\r\n", getServerUri());
        f.format("To: %s\r\n", uri);
        f.format("Date: %s\r\n", m_dateFormat.format(new Date()));
        f.format("Call-ID: 90d3f2-%x\r\n", generateUniqueId());
        header.append("CSeq: 1 NOTIFY\r\n");
        f.format("Contact: %s\r\n", getContactUri());
        header.append(event);
        f.format("Content-Length: %d\r\n\r\n", payloadLen);
    }

    /**
     * Doesn't include Display name or angle bracket, e.g. sip:user@blah.com, not "User
     * Name"&lt;sip:user@blah.com&gt;
     * 
     * NOTE: Unlike request URIs for REGISTER, this apparently requires the user portion.
     * 
     * NOTE: I found this out thru trial and error.
     */
    String getNotifyRequestUri(String userId, String host, int portNumber) {
        int port = (portNumber == SipUri.DEFAULT_SIP_PORT) ? SipUri.OMIT_SIP_PORT : portNumber;
        return SipUri.format(userId, host, port);
    }

    String getContactUri() {
        return SipUri.format(m_serverName, m_proxyPort);
    }

    private long generateUniqueId() {
        return RandomUtils.nextLong();
    }
}
