/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.io.IOException;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.text.NumberFormat;

import org.apache.commons.net.ntp.*;

import org.sipfoundry.commons.dhcp.NetworkResources;
import org.sipfoundry.commons.util.JournalService;

import static org.sipfoundry.preflight.ResultCode.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class NTP {
	private static final int bindPort = 0;
    private static final NumberFormat numberFormat = new java.text.DecimalFormat("0.00");

    public ResultCode validate(int timeout, NetworkResources networkResources, JournalService journalService, InetAddress bindAddress) {
        ResultCode results = NONE;

        if (networkResources.ntpServers == null) {
            journalService.println("No NTP servers provided, skipping test.");
            results = NTP_SERVERS_MISSING;
        } else {
            journalService.println("Starting NTP servers test.");
            NTPUDPClient client = new NTPUDPClient();
            client.setDefaultTimeout(timeout * 1000);
            try {
            	client.open(bindPort, bindAddress);

                for (InetAddress ntpServer : networkResources.ntpServers) {
                    journalService.println("NTP test for server: " + ntpServer.getCanonicalHostName());
                    try {
                        TimeInfo info = client.getTime(ntpServer);
                        journalService.println(new String(processResponse(info)));
                    } catch (IOException e) {
                        client.close();
                        if (e.getClass() == java.net.SocketTimeoutException.class) {
                            journalService.println("  NTP request timed out.");
                            results = NTP_SERVER_FAILURE;
                        } else {
                            journalService.println("  NTP request error: " + e.getMessage());
                            results = NTP_TEST_FAILURE;
                        }
                    }
                }
            } catch (SocketException e) {
                journalService.println("Unable to create NTP client: " + e.getMessage());
                results = NTP_TEST_FAILURE;
            }
        }

        journalService.println("");
        return results;
    }

    /**
     * Process <code>TimeInfo</code> object and return details.
     *
     * @param info
     *            <code>TimeInfo</code> object.
     */
    private String processResponse(TimeInfo info) {
        String response;
        NtpV3Packet message = info.getMessage();
        int stratum = message.getStratum();
        String refType;
        if (stratum <= 0) {
            refType = "(Unspecified or Unavailable)";
        } else if (stratum == 1) {
            refType = "(Primary Reference; e.g., GPS)"; // GPS, radio clock,
                                                        // etc.
        } else {
            refType = "(Secondary Reference; e.g. via NTP or SNTP)";
        }
        // stratum should be 0..15...
        response = new String("  Stratum: " + stratum + " " + refType + "\n");
        int version = message.getVersion();
        int li = message.getLeapIndicator();
        response += new String("  leap=" + li + ", version=" + version + ", precision=" + message.getPrecision() + "\n");

        response += new String("  mode: " + message.getModeName() + " (" + message.getMode() + ")\n");
        int poll = message.getPoll();
        // poll value typically btwn MINPOLL (4) and MAXPOLL (14)
        response += new String("  poll: " + (poll <= 0 ? 1 : (int) Math.pow(2, poll)) + " seconds" + " (2 ** " + poll + ")\n");
        double disp = message.getRootDispersionInMillisDouble();
        response += new String("  rootdelay=" + numberFormat.format(message.getRootDelayInMillisDouble())
                + ", rootdispersion(ms): " + numberFormat.format(disp) + "\n");

        int refId = message.getReferenceId();
        String refAddr = NtpUtils.getHostAddress(refId);
        String refName = null;
        if (refId != 0) {
            if (refAddr.equals("127.127.1.0")) {
                refName = "LOCAL"; // This is the ref address for the Local
                                    // Clock
            } else if (stratum >= 2) {
                // If reference id has 127.127 prefix then it uses its own
                // reference clock
                // defined in the form 127.127.clock-type.unit-num (e.g.
                // 127.127.8.0 mode 5
                // for GENERIC DCF77 AM; see refclock.htm from the NTP software
                // distribution.
                if (!refAddr.startsWith("127.127")) {
                    try {
                        InetAddress addr = InetAddress.getByName(refAddr);
                        String name = addr.getHostName();
                        if (name != null && !name.equals(refAddr)) {
                            refName = name;
                        }
                    } catch (UnknownHostException e) {
                        // some stratum-2 servers sync to ref clock device but
                        // fudge stratum level higher... (e.g. 2)
                        // ref not valid host maybe it's a reference clock name?
                        // otherwise just show the ref IP address.
                        refName = NtpUtils.getReferenceClock(message);
                    }
                }
            } else if (version >= 3 && (stratum == 0 || stratum == 1)) {
                refName = NtpUtils.getReferenceClock(message);
                // refname usually have at least 3 characters (e.g. GPS, WWV,
                // LCL, etc.)
            }
            // otherwise give up on naming the beast...
        }
        if (refName != null && refName.length() > 1) {
            refAddr += " (" + refName + ")";
        }
        response += new String("  Reference Identifier:  " + refAddr + "\n");

        TimeStamp refNtpTime = message.getReferenceTimeStamp();
        response += new String("  Reference Timestamp:   " + refNtpTime + "  " + refNtpTime.toDateString() + "\n");

        // Originate Time is time request sent by client (t1)
        TimeStamp origNtpTime = message.getOriginateTimeStamp();
        response += new String("  Originate Timestamp:   " + origNtpTime + "  " + origNtpTime.toDateString() + "\n");

        long destTime = info.getReturnTime();
        // Receive Time is time request received by server (t2)
        TimeStamp rcvNtpTime = message.getReceiveTimeStamp();
        response += new String("  Receive Timestamp:     " + rcvNtpTime + "  " + rcvNtpTime.toDateString() + "\n");

        // Transmit time is time reply sent by server (t3)
        TimeStamp xmitNtpTime = message.getTransmitTimeStamp();
        response += new String("  Transmit Timestamp:    " + xmitNtpTime + "  " + xmitNtpTime.toDateString() + "\n");

        // Destination time is time reply received by client (t4)
        TimeStamp destNtpTime = TimeStamp.getNtpTime(destTime);
        response += new String("  Destination Timestamp: " + destNtpTime + "  " + destNtpTime.toDateString() + "\n");

        info.computeDetails(); // compute offset/delay if not already done
        Long offsetValue = info.getOffset();
        Long delayValue = info.getDelay();
        String delay = (delayValue == null) ? "N/A" : delayValue.toString();
        String offset = (offsetValue == null) ? "N/A" : offsetValue.toString();

        response += new String("  Roundtrip delay(ms)=" + delay + ", clock offset(ms)=" + offset + "\n"); // offset
                                                                                                            // in
                                                                                                            // ms

        return response;
    }

}
