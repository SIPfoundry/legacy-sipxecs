package org.sipfoundry.sipxconfig.sip;

import java.io.IOException;
import java.util.Formatter;
import java.util.Locale;

/**
 * The check-sync message is a flavor of unsolicited NOTIFY (no subscription required), the
 * message allows us to reboot a specific phone. It is sent to the user registered for that phone.
 * 
 */
class NotifyMessage extends Message {
    private String m_addrSpec;
    private String m_eventType;

    public NotifyMessage(String serverName, String addSpec, String eventType, String contentType, byte[] payload) {
        super(serverName, contentType, payload);
        m_addrSpec = addSpec;
        m_eventType = eventType;
    }

    public NotifyMessage(String serverName, String addSpec, String eventType) {
        super(serverName);
        m_addrSpec = addSpec;
        m_eventType = eventType;
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
        f.format("Event: %s\r\n", m_eventType);
        addCommonHeaders(f, proxyHost, proxyPort);
    }
}
