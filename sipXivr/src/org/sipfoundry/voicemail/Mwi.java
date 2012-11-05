/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.voicemail;

import java.io.UnsupportedEncodingException;
import java.net.URL;
import java.net.URLEncoder;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.util.RemoteRequest;
import org.sipfoundry.voicemail.mailbox.MailboxDetails;


public class Mwi {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    public static final String MessageSummaryContentType = "application/simple-message-summary";
    private static final String MWI_URL = "http://%s:%s/cgi/StatusEvent.cgi";
    private String[] m_mwiAddresses;
    private String m_mwiPort;

    /**
     * Format the status ala RFC-3842
     *
     * @param numNew
     * @param numOld
     * @param numNewUrgent
     * @param numOldUrgent
     * @return
     */
    public static String formatRFC3842(int numNew, int numOld, int numNewUrgent, int numOldUrgent, String accountUrl) {
        return String.format("Messages-Waiting: %s\r\nMessage-Account: %s\r\nVoice-Message: %d/%d (%d/%d)\r\n\r\n",
                numNew > 0 ? "yes":"no", accountUrl, numNew, numOld, numNewUrgent, numOldUrgent);
    }

    public static String formatRFC3842(MailboxDetails messages, String accountUrl) {
        return formatRFC3842(messages.getUnheardCount(), messages.getHeardCount(), 0, 0, accountUrl);
    }

    /**
     * Send MWI info to the Status Server (which in turn sends it to interested parties via SIP NOTIFY)
     * @param mailbox
     * @param messages
     */
    public void sendMWI(User user, MailboxDetails mailbox) {
        String idUri = user.getIdentity();

        String accountUrl = "sip:" + idUri;
        try {
            String content = "identity=" +
                    URLEncoder.encode(idUri, "UTF-8") + "&eventType=message-summary&event-data=" + "\r\n" +
                    formatRFC3842(mailbox, accountUrl);
            for (String mwiAddress : m_mwiAddresses) {
                LOG.info(String.format("Mwi::SendMWI %s to MWI address %s", idUri, mwiAddress));
                if (sendMwi(mwiAddress, content)) {
                    break;
                }
            }
        } catch (UnsupportedEncodingException ex) {
            LOG.error("Mwi::sendMWI Trouble with encoding idUri", ex);
        }
    }

    private boolean sendMwi(String mwiAddress, String content) {
        try {
            String mwiApiUrl = String.format(MWI_URL, mwiAddress, m_mwiPort);
            URL mwiUrl = new URL(mwiApiUrl);
            RemoteRequest rr = new RemoteRequest(mwiUrl, MessageSummaryContentType, content);
            boolean result = rr.http();
            if (!result) {
                LOG.error("Mwi::sendMWI Trouble with RemoteRequest on address "+rr.getResponse());
            }
            return result;
        } catch (Exception e) {
            LOG.error("Mwi::sendMWI Trouble with mwiUrl", e);
            return false;
        }
    }

    public void setMwiAddresses(String addresses) {
        m_mwiAddresses = StringUtils.split(addresses, ",");
    }

    public void setMwiPort(String port) {
        m_mwiPort = port;
    }
}
