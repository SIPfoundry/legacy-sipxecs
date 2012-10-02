/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.voicemail;

import java.net.URL;
import java.net.URLEncoder;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.util.RemoteRequest;
import org.sipfoundry.voicemail.mailbox.MailboxDetails;


public class Mwi {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    public static final String MessageSummaryContentType = "application/simple-message-summary";
    private String m_mwiUrl;

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

        LOG.info(String.format("Mwi::SendMWI %s", idUri));

        // URL of Status Server
        URL mwiUrl;
        try {
            mwiUrl = new URL(m_mwiUrl);
            String accountUrl = "sip:" + idUri;
            String content = "identity=" +
                URLEncoder.encode(idUri, "UTF-8") + "&eventType=message-summary&event-data=" + "\r\n" +
                formatRFC3842(mailbox, accountUrl);
            RemoteRequest rr = new RemoteRequest(mwiUrl, MessageSummaryContentType, content);
            if (!rr.http()) {
                LOG.error("Mwi::sendMWI Trouble with RemoteRequest "+rr.getResponse());
            }
        } catch (Exception e) {
            LOG.error("Mwi::sendMWI Trouble with mwiUrl", e);
        }
    }

    public void setMwiUrl(String url) {
        m_mwiUrl = url;
    }
}
