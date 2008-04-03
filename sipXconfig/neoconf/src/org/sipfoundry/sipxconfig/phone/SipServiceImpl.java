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

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.DateFormat;
import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.SipUri;


/**
 * Implementation note: This is very, very basic sip implementation. Going forward, either
 * use sipXtack java binding or interact w/sipXpublisher thru some RPC to have it deliver
 * sip messages on behalf or sipXconfig
 */
public class SipServiceImpl implements SipService {
    
    private static final long RANDOM_MAX = Long.MAX_VALUE;
    
    private static final Log LOG = LogFactory.getLog(SipServiceImpl.class);

    private static final String COLON = ":";

    /** Example:  Tue, 15 Nov 1994 08:12:31 GMT */
    private SimpleDateFormat m_dateFormat;

    private String m_serverName = "localhost";

    private int m_serverPort = SipUri.DEFAULT_SIP_PORT;
    
    private String m_proxyHost = m_serverName;
    
    private int m_proxyPort = SipUri.DEFAULT_SIP_PORT;
    
    public SipServiceImpl() {
        m_dateFormat = new SimpleDateFormat("EEE, d MMM yyyy kk:mm:ss z");
        m_dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        
        try {
            m_serverName = InetAddress.getLocalHost().getHostAddress();
        } catch (UnknownHostException nonFatal) {
            // For now, we never read response and could care less if return
            // address is unattainable.  Next iteration of this SipService
            // may use a real stack w/access to config.defs.
            LOG.warn("cannot determine server name", nonFatal);
        }
    }

    public void setProxyHost(String proxy) {
        m_proxyHost = proxy;
    }

    public void setProxyPort(int port) {
        m_proxyPort = port;
    }

    public String getServerVia() {
        return "SIP/2.0/UDP " + getFromServerName() + COLON + getFromServerPort() + ";branch="
                + generateBranchId();
    }

    public String generateBranchId() {
        return generateUniqueId();
    }

    public String getFromServerName() {
        return m_serverName;
    }

    public void setFromServerName(String serverName) {
        m_serverName = serverName;
    }

    public int getFromServerPort() {
        return m_serverPort;
    }

    public void setFromServerPort(int serverPort) {
        m_serverPort = serverPort;
    }
    
    private String getFromServer() {
        String from;
        if (getFromServerPort() == SipUri.DEFAULT_SIP_PORT) {
            from  = getFromServerName();
        } else {
            from  = getFromServerName() + COLON + getFromServerPort();
        }
        
        return from;
    }

    public String getServerUri() {        
        return "sip:sipuaconfig@" + getFromServer();
    }

    public void send(byte[] sipBytes) throws IOException {
        // not particular reason it's UDP other than we do not
        // expect a response so this seems more appropriate.
        LOG.info(new String(sipBytes));
        DatagramSocket socket = new DatagramSocket();
        InetAddress toAddress = InetAddress.getByName(m_proxyHost);
        DatagramPacket packet = new DatagramPacket(sipBytes, 0, sipBytes.length, 
                toAddress, m_proxyPort);
        socket.send(packet);
        socket.disconnect();
    }
    
    public String getCurrentDate() {
        return getDateFormat().format(new Date());
    }

    public DateFormat getDateFormat() {
        return m_dateFormat;
    }

    public String getContactUri() {
        return SipUri.SIP_PREFIX + getFromServerName() + COLON + getFromServerPort();
    }

    public String generateCallId() {
        return "90d3f2-" + generateUniqueId();
    }

    String generateUniqueId() {
        long l = (long) (Math.random() * RANDOM_MAX);
        return Long.toHexString(l);
    }
    
    public void sendCheckSync(String uri, String server, String port) {
        sendNotify(uri, server, port, "Event: check-sync\r\n", new byte[0]);
    }

    public void sendNotify(String uri, String server, String port, String event, byte[] payload) {
        String userId = SipUri.extractUser(uri);
        // The check-sync message is a flavor of unsolicited NOTIFY
        // this message does not require that the phone be enrolled
        // the message allows us to reboot a specific phone 
        String restartSip = "NOTIFY {0} SIP/2.0\r\n" + "Via: {1}\r\n"
                + "From: {2}\r\n" + "To: {3}\r\n"
                + "Date: {4}\r\n" + "Call-ID: {5}\r\n" + "CSeq: 1 NOTIFY\r\n"
                + "Contact: {6}\r\n"
                + event
                + "Content-Length: {7}\r\n" + "\r\n";
        Object[] sipParams = new Object[] { 
            getNotifyRequestUri(server, port, userId), 
            getServerVia(),
            getServerUri(), 
            uri,
            getCurrentDate(), 
            generateCallId(),
            getContactUri(),
            String.valueOf(payload.length)
        };
        String header = MessageFormat.format(restartSip, sipParams);
        int headerLen = header.length();
        byte[] msg = new byte[headerLen + payload.length];
        System.arraycopy(header.getBytes(), 0, msg, 0, headerLen);
        System.arraycopy(payload, 0, msg, headerLen, payload.length);
        try {
            send(msg);
        } catch (IOException e) {
            throw new RestartException("Could not send restart SIP message", e);
        }        
        
    }

    /**
     * Doesn't include Display name or angle bracket, 
     * e.g. sip:user@blah.com, not "User Name"&lt;sip:user@blah.com&gt; 
     * NOTE: Unlike request URIs for REGISTER, this apparently requires the user
     * portion.  NOTE: I found this out thru trial and error.
     */
    String getNotifyRequestUri(String registrationServer, String registrationServerPort, String userId) {
        StringBuffer sb = new StringBuffer();        
        sb.append(SipUri.SIP_PREFIX).append(userId);
        // TODO: Should this be outbound proxy
        sb.append('@').append(registrationServer);
        String port = registrationServerPort;
        if (StringUtils.isNotBlank(port)) {
            int nport = Integer.parseInt(port);
            if (nport != SipUri.DEFAULT_SIP_PORT) {
                sb.append(':').append(port);
            }
        }
        
        return sb.toString();        
    }
}
