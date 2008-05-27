package org.sipfoundry.sipxconfig.sip;

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

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.CharEncoding;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.device.RestartException;

/**
 * Utility class for formatting headers in SIP message.
 */
abstract class Message extends AbstractMessage {
    private String m_serverName;
    private byte[] m_payload;
    private String m_contentType;

    public Message(String serverName, String contentType, byte[] payload) {
        m_serverName = serverName;
        m_contentType = contentType;
        m_payload = payload;
    }

    public Message(String serverName) {
        this(serverName, null, ArrayUtils.EMPTY_BYTE_ARRAY);
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

    protected final void send(String proxyHost, int proxyPort, byte[] sipBytes) throws IOException {
        SipServiceImpl.LOG.info(new String(sipBytes, CharEncoding.US_ASCII));
        // not particular reason it's UDP other than we do not expect a response
        DatagramSocket socket = new DatagramSocket();
        InetAddress toAddress = InetAddress.getByName(proxyHost);
        DatagramPacket packet = new DatagramPacket(sipBytes, 0, sipBytes.length, toAddress, proxyPort);
        socket.send(packet);
        socket.disconnect();
    }

    abstract void formatHeaders(Appendable buf, String proxyHost, int proxyPort) throws IOException;

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
        if (m_contentType != null) {
            f.format("Content-Type: %d\r\n\r\n", m_contentType);
        }
    }
}
