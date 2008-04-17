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
import java.util.Calendar;
import java.util.Formatter;
import java.util.Locale;
import java.util.TimeZone;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.CharEncoding;
import org.apache.commons.lang.math.RandomUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.springframework.beans.factory.annotation.Required;

/**
 * Implementation note: This is very, very basic sip implementation. Going forward, either use
 * sipXtack java binding or interact w/sipXpublisher thru some RPC to have it deliver sip messages
 * on behalf or sipXconfig
 */
public class SipServiceImpl implements SipService {

    private static final Log LOG = LogFactory.getLog(SipServiceImpl.class);

    private static final TimeZone GMT = TimeZone.getTimeZone("GMT");

    private String m_serverName;

    private String m_proxyHost;

    private int m_proxyPort = SipUri.DEFAULT_SIP_PORT;

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

    public void sendCheckSync(String addrSpec) {
        sendNotify(addrSpec, "Event: check-sync\r\n", ArrayUtils.EMPTY_BYTE_ARRAY);
    }

    public void sendNotify(String addrSpec, String event, byte[] payload) {
        try {
            ByteArrayOutputStream msgStream = new ByteArrayOutputStream();
            Writer msgWriter = new OutputStreamWriter(msgStream, CharEncoding.US_ASCII);
            formatHeaders(msgWriter, addrSpec, event, payload.length);
            msgWriter.flush();
            msgStream.write(payload);
            send(msgStream.toByteArray());
            msgStream.close();
        } catch (IOException e) {
            throw new RestartException("Could not send restart SIP message", e);
        }
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

    /**
     * Format headers for check-sync notification
     * 
     * The check-sync message is a flavor of unsolicited NOTIFY (no subscription required), the
     * message allows us to reboot a specific phone. It is sent to the user registered for that
     * phone.
     * 
     * @param buf buffer into which the message headers are formatter
     * @param addSpec usually user short version of SIP URI (no display name)
     * @param event additional headers (usually just Event header)
     * @param payloadLen length of the payload (in bytes)
     */
    void formatHeaders(Appendable buf, String addSpec, String event, int payloadLen) throws IOException {
        long uniqueId = generateUniqueId();
        Formatter f = new Formatter(buf, Locale.US);
        f.format("NOTIFY %s SIP/2.0\r\n", addSpec);
        f.format("Via: SIP/2.0/UDP %s:%d;branch=%x\r\n", m_proxyHost, m_proxyPort, uniqueId);
        String from = SipUri.format("sipuaconfig", m_serverName, SipUri.OMIT_SIP_PORT);
        f.format("From: %s\r\n", from);
        f.format("To: %s\r\n", addSpec);

        Calendar c = Calendar.getInstance(GMT, Locale.US);
        // date format Tue, 15 Nov 1994 08:12:31 GMT
        f.format("Date: %1$ta, %1$td %1$tb %1$tY %1$tH:%1$tM:%1$tS GMT\r\n", c);
        f.format("Call-ID: 90d3f2-%x\r\n", uniqueId);
        buf.append("CSeq: 1 NOTIFY\r\n");
        String contact = SipUri.format(m_proxyHost, m_proxyPort);
        f.format("Contact: %s\r\n", contact);
        buf.append(event);
        f.format("Content-Length: %d\r\n\r\n", payloadLen);
    }

    private long generateUniqueId() {
        return RandomUtils.nextLong();
    }
}
