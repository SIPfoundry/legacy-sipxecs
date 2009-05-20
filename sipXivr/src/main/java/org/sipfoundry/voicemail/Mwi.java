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

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.RemoteRequest;


public class Mwi {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private static Mwi s_me;
    public static final String MessageSummaryContentType = "application/simple-message-summary";

    boolean m_justTesting;

    private Mwi(boolean justTesting) {
        m_justTesting = justTesting;
    }
    
    public static Mwi getMwi() {
        if (s_me == null) {
            s_me = new Mwi(false);
        }
        return s_me;
    }
    
    /**
     * Format the status ala RFC-3842
     * 
     * @param numNew
     * @param numOld
     * @param numNewUrgent
     * @param numOldUrgent
     * @return
     */
    public static String formatRFC3842(int numNew, int numOld, int numNewUrgent, int numOldUrgent) {
        return String.format("Messages-Waiting: %s\r\nVoice-Message: %d/%d (%d/%d)\r\n\r\n",
                numNew > 0 ? "yes":"no", numNew, numOld, numNewUrgent, numOldUrgent);
    }

    /**
     * Send MWI info to the Status Server (which in turn sends it to interested parties via SIP NOTIFY)
     * (Loads up messages)
     * @param mailbox
     */
    public static void sendMWI(Mailbox mailbox) {
        Messages messages = new Messages(mailbox);
        sendMWI(mailbox, messages);
    }
    
    /**
     * Send MWI info to the Status Server (which in turn sends it to interested parties via SIP NOTIFY)
     * @param mailbox
     * @param messages
     */
    public static void sendMWI(Mailbox mailbox, Messages messages) {
        if (Mwi.isJustTesting()) {
            // Don't do anything if we are just in a test situation
            return ;
        }

        int heard = 0;
        int unheard = 0;
        int heardUrgent = 0;
        int unheardUrgent = 0;
        String idUri = mailbox.getUser().getIdentity();

        heard = messages.getHeardCount();
        unheard = messages.getUnheardCount();
        // No support for urgent messages at this time

        LOG.info(String.format("Mwi::SendMWI %s %d/%d", idUri, unheard, heard));
        // URL of Status Server
        String mwiUrlString = org.sipfoundry.sipxivr.Configuration.get().getMwiUrl();
        URL mwiUrl;
        try {
            mwiUrl = new URL(mwiUrlString);
            String content = "eventType=message-summary&" + "identity=" + idUri + "\r\n" + 
            formatRFC3842(unheard, heard, unheardUrgent, heardUrgent);
            RemoteRequest rr = new RemoteRequest(mwiUrl, MessageSummaryContentType, content);
            if (!rr.http()) {
                LOG.error("Mwi::sendMWI Trouble with RemoteRequest "+rr.getResponse());
            }
        } catch (Exception e) {
            LOG.error("Mwi::sendMWI Trouble with mwiUrl", e);
        }
    }

    public static boolean isJustTesting() {
        return getMwi().m_justTesting;
    }

    public static void setJustTesting(boolean justTesting) {
        getMwi().m_justTesting = justTesting;
    }
}
