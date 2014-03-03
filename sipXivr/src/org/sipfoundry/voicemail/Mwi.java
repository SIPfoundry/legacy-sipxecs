/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.voicemail;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URL;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.util.RemoteRequest;
import org.sipfoundry.voicemail.mailbox.MailboxDetails;

public class Mwi {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    public static final String MessageSummaryContentType = "application/simple-message-summary";
    private static final String MWI_URL = "http://%s:%s/cgi/StatusEvent.cgi";
    private Map<String, List<String>> m_mwiAddresses;
    private String m_mwiPort;
    private int m_mwiTimeout = 5;

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
                numNew > 0 ? "yes" : "no", accountUrl, numNew, numOld, numNewUrgent, numOldUrgent);
    }

    public static String formatRFC3842(MailboxDetails messages, String accountUrl) {
        return formatRFC3842(messages.getUnheardCount(), messages.getHeardCount(), 0, 0, accountUrl);
    }

    /**
     * Send MWI info to the Status Server (which in turn sends it to interested parties via SIP
     * NOTIFY)
     * 
     * @param mailbox
     * @param messages
     */
    public void sendMWI(User user, final MailboxDetails mailbox) {
        final String idUri = user.getIdentity();

        String accountUrl = "sip:" + idUri;
        try {
            final String content = "identity=" + URLEncoder.encode(idUri, "UTF-8")
                    + "&eventType=message-summary&event-data=" + "\r\n" + formatRFC3842(mailbox, accountUrl);

            for (final String region : m_mwiAddresses.keySet()) {
                sendMwiToRegion(region, idUri, mailbox, content);
            }

        } catch (UnsupportedEncodingException ex) {
            LOG.error("Mwi::sendMWI Trouble with encoding idUri", ex);
        }
    }

    private void sendMwiToRegion(final String region, final String idUri, final MailboxDetails mailbox,
            final String content) {
        try {
            ExecutorService executor = Executors.newSingleThreadExecutor();
            Callable<Boolean> task = new Callable<Boolean>() {
                public Boolean call() throws IOException {
                    List<String> addressesInRegion = m_mwiAddresses.get(region);
                    for (String mwiAddress : addressesInRegion) {
                        LOG.info(String.format(
                                "Mwi::SendMWI %s %d/%d to MWI address %s in region %s with timeout %d", idUri,
                                mailbox.getUnheardCount(), mailbox.getHeardCount(), mwiAddress, region, m_mwiTimeout));
                        if (sendMwi(mwiAddress, content)) {
                            return true;
                        }
                    }
                    return false;
                }
            };
            final List<Future<Boolean>> futures = executor.invokeAll(Arrays.asList(task), m_mwiTimeout,
                    TimeUnit.SECONDS);
            executor.shutdown();
            if (futures.get(0).isCancelled()) {
                LOG.error(String.format("Mwi::SendMWI to region %s timed out", region));
            }
        } catch (InterruptedException e) {
            LOG.error("Unexpected termination of sending MWI action", e);
        }
    }

    private boolean sendMwi(String mwiAddress, String content) {
        try {
            String mwiApiUrl = String.format(MWI_URL, mwiAddress, m_mwiPort);
            URL mwiUrl = new URL(mwiApiUrl);
            RemoteRequest rr = new RemoteRequest(mwiUrl, MessageSummaryContentType, content);
            boolean result = rr.http();
            if (!result) {
                LOG.error("Mwi::sendMWI Trouble with RemoteRequest on address " + rr.getResponse());
            }
            return result;
        } catch (Exception e) {
            LOG.error("Mwi::sendMWI Trouble with mwiUrl", e);
            return false;
        }
    }

    public void setMwiAddresses(String addresses) {
        Map<String, List<String>> addrs = new LinkedHashMap<String, List<String>>();
        String[] addressesWithRegion = StringUtils.split(addresses, ",");
        for (String address : addressesWithRegion) {
            if (address.contains("@")) {
                String[] addrValues = StringUtils.split(address, "@");
                String ip = addrValues[0];
                String region = addrValues[1];
                List<String> addrsPerRegion = addrs.get(region);
                if (addrsPerRegion == null) {
                    addrsPerRegion = new ArrayList<String>();
                }
                addrsPerRegion.add(ip);
                addrs.put(region, addrsPerRegion);
            } else {
                List<String> globalAddrs = addrs.get("global");
                if (globalAddrs == null) {
                    globalAddrs = new ArrayList<String>();
                }
                globalAddrs.add(address);
                addrs.put("global", globalAddrs);
            }
        }
        m_mwiAddresses = addrs;
    }

    public Map<String, List<String>> getSortedAddresses() {
        return m_mwiAddresses;
    }

    public void setMwiPort(String port) {
        m_mwiPort = port;
    }

    public void setMwiTimeout(int timeout) {
        m_mwiTimeout = timeout;
    }
}
