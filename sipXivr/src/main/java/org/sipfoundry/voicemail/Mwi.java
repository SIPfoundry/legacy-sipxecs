/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.voicemail;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Mailbox;


public class Mwi {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private static Mwi m_me;
    public static final String MessageSummaryContentType = "application/simple-message-summary";

    boolean m_justTesting;

    private Mwi(boolean justTesting) {
        m_justTesting = justTesting;
    }
    
    public static Mwi getMwi() {
        if (m_me == null) {
            m_me = new Mwi(false);
        }
        return m_me;
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
        // No support for urgent messages

        LOG.info(String.format("Mwi::SendMWI %s %d/%d", idUri, unheard, heard));
        try {
            // URL of Status Server
            String mwiUrlString = org.sipfoundry.sipxivr.Configuration.get().getMwiUrl();
            URL mwiUrl = new URL(mwiUrlString);
            DataOutputStream printout;
            DataInputStream input;

            // URL connection channel.
            HttpURLConnection urlConn = (HttpURLConnection) mwiUrl.openConnection();

            // Let the run-time system (RTS) know that we want input.
            urlConn.setDoInput(true);

            // Let the RTS know that we want to do output.
            urlConn.setDoOutput(true);

            // No caching, we want the real thing.
            urlConn.setUseCaches(false);

            // Specify the content type.
            urlConn.setRequestProperty("Content-Type", MessageSummaryContentType);


            String content = "eventType=message-summary&" + "identity=" + idUri + "\r\n" + 
                formatRFC3842(unheard, heard, unheardUrgent, heardUrgent);
            LOG.debug("Mwistatus::SendMWI posting "+content);
            printout = new DataOutputStream(urlConn.getOutputStream());

            printout.writeBytes(content);
            printout.flush();
            printout.close();

            input = new DataInputStream(urlConn.getInputStream());

            input.close();
        }

        catch (Exception ex) {
            LOG.error("MwiStatus::SendMWI trouble", ex);
        }
    }

    public static boolean isJustTesting() {
        return getMwi().m_justTesting;
    }

    public static void setJustTesting(boolean justTesting) {
        getMwi().m_justTesting = justTesting;
    }
}
