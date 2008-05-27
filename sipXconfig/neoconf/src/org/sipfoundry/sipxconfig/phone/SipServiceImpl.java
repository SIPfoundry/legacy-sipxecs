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
import org.sipfoundry.sipxconfig.device.RestartException;
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
        Message message = new NotifyMessage(m_serverName, addrSpec, "Event: check-sync\r\n");
        message.createAndSend(m_proxyHost, m_proxyPort);
    }

    public void sendNotify(String addrSpec, String event, byte[] payload) {
        Message message = new NotifyMessage(m_serverName, addrSpec, event, payload);
        message.createAndSend(m_proxyHost, m_proxyPort);
    }

    public void sendRefer(String sourceAddrSpec, String destinationAddSpec) {
        Message message = new ReferMessage(m_serverName, sourceAddrSpec, destinationAddSpec);
        message.createAndSend(m_proxyHost, m_proxyPort);
    }

    /**
     * Utility class for formatting headers in SIP message.
     */
    abstract static class Message {
        private String m_serverName;
        private byte[] m_payload;

        public Message(String serverName, byte[] payload) {
            m_serverName = serverName;
            m_payload = payload;
        }

        public Message(String serverName) {
            this(serverName, ArrayUtils.EMPTY_BYTE_ARRAY);
        }

        public final void createAndSend(String proxyHost, int proxyPort) {
            try {
                ByteArrayOutputStream msgStream = new ByteArrayOutputStream();
                Writer msgWriter = new OutputStreamWriter(msgStream, CharEncoding.US_ASCII);
                formatHeaders(msgWriter, proxyHost, proxyPort);
                msgWriter.flush();
                msgStream.write(m_payload);
                send(proxyHost, proxyPort, msgStream.toByteArray());
                msgStream.close();
            } catch (IOException e) {
                throw new RestartException("Could not send restart SIP message", e);
            }
        }

        final void send(String proxyHost, int proxyPort, byte[] sipBytes) throws IOException {
            LOG.info(new String(sipBytes, CharEncoding.US_ASCII));
            // not particular reason it's UDP other than we do not expect a response
            DatagramSocket socket = new DatagramSocket();
            InetAddress toAddress = InetAddress.getByName(proxyHost);
            DatagramPacket packet = new DatagramPacket(sipBytes, 0, sipBytes.length, toAddress, proxyPort);
            socket.send(packet);
            socket.disconnect();
        }

        abstract void formatHeaders(Appendable buf, String proxyHost, int proxyPort) throws IOException;

        protected long generateUniqueId() {
            return RandomUtils.nextLong();
        }

        protected String getFromHeader() {
            return SipUri.format("sipuaconfig", m_serverName, SipUri.OMIT_SIP_PORT);
        }

        protected void addVia(Formatter f, String proxyHost, int proxyPort, long uniqueId) {
            f.format("Via: SIP/2.0/UDP %s:%d;branch=%x\r\n", proxyHost, proxyPort, uniqueId);
        }

        protected void addTo(Formatter f, String toUri) {
            f.format("To: %s\r\n", toUri);
        }

        protected void addFrom(Formatter f) {
            f.format("From: %s\r\n", getFromHeader());
        }

        protected void addCommonHeaders(Formatter f, String proxyHost, int proxyPort) {
            Calendar c = Calendar.getInstance(GMT, Locale.US);
            // date format Tue, 15 Nov 1994 08:12:31 GMT
            f.format("Date: %1$ta, %1$td %1$tb %1$tY %1$tH:%1$tM:%1$tS GMT\r\n", c);
            String contact = SipUri.format(proxyHost, proxyPort);
            f.format("Contact: %s\r\n", contact);
            f.format("Content-Length: %d\r\n\r\n", m_payload.length);
        }
    }

    /**
     * The check-sync message is a flavor of unsolicited NOTIFY (no subscription required), the
     * message allows us to reboot a specific phone. It is sent to the user registered for that
     * phone.
     * 
     */
    static class NotifyMessage extends Message {
        private String m_addrSpec;
        private String m_event;

        public NotifyMessage(String serverName, String addSpec, String event, byte[] payload) {
            super(serverName, payload);
            m_addrSpec = addSpec;
            m_event = event;
        }

        public NotifyMessage(String serverName, String addSpec, String event) {
            super(serverName);
            m_addrSpec = addSpec;
            m_event = event;
        }

        /**
         * Format headers for check-sync notification
         * 
         * @param buf buffer into which the message headers are formatter
         * @param addSpec usually user short version of SIP URI (no display name)
         * @param event additional headers (usually just Event header)
         * @param payloadLen length of the payload (in bytes)
         */
        void formatHeaders(Appendable buf, String proxyHost, int proxyPort) throws IOException {
            long uniqueId = generateUniqueId();
            Formatter f = new Formatter(buf, Locale.US);
            f.format("NOTIFY %s SIP/2.0\r\n", m_addrSpec);
            addVia(f, proxyHost, proxyPort, uniqueId);
            addTo(f, m_addrSpec);
            addFrom(f);
            f.format("Call-ID: 90d3f2-%x\r\n", uniqueId);
            buf.append("CSeq: 1 NOTIFY\r\n");
            buf.append(m_event);
            addCommonHeaders(f, proxyHost, proxyPort);
        }
    }

    static class ReferMessage extends Message {

        private String m_sourceAddrSpec;
        private String m_destinationAddSpec;

        public ReferMessage(String serverName, String sourceAddrSpec, String destinationAddrSpec) {
            super(serverName);
            m_sourceAddrSpec = sourceAddrSpec;
            m_destinationAddSpec = destinationAddrSpec;
        }

        void formatHeaders(Appendable buf, String proxyHost, int proxyPort) throws IOException {
            long uniqueId = generateUniqueId();
            Formatter f = new Formatter(buf, Locale.US);
            f.format("REFER %s SIP/2.0\r\n", m_sourceAddrSpec);
            addVia(f, proxyHost, proxyPort, uniqueId);
            addTo(f, m_sourceAddrSpec);
            addFrom(f);
            f.format("Call-ID: 90d3f3-%x\r\n", uniqueId);
            buf.append("CSeq: 1 REFER\r\n");
            f.format("Refer-To: %s\r\n", m_destinationAddSpec);
            addCommonHeaders(f, proxyHost, proxyPort);
        }
    }
}
