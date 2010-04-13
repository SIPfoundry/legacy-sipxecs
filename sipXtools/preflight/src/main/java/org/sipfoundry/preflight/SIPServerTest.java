/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.net.InetAddress;

import org.sipfoundry.commons.dhcp.NetworkResources;
import org.sipfoundry.commons.dhcp.SIPServer;
import org.sipfoundry.commons.discovery.DiscoveryService;
import org.sipfoundry.commons.icmp.Pinger;
import org.sipfoundry.commons.util.JournalService;

import static org.sipfoundry.preflight.ResultCode.*;

/**
 * [Enter descriptive text here]
 * <p>
 * 
 * @author Mardy Marshall
 */
public class SIPServerTest {
    public ResultCode validate(int timeout, NetworkResources networkResources, JournalService journalService, InetAddress bindAddress) {
        ResultCode results = SIP_SERVER_UNREACHABLE;
        Pinger pinger;
        String targetAddress;

        if (networkResources.sipServers == null) {
            journalService.println("No SIP servers provided, skipping test.");
            results = SIP_SERVERS_MISSING;
        } else {
            journalService.println("Starting SIP servers test.");
            for (SIPServer server : networkResources.sipServers) {
            	if (server.IPaddress != null) {
            		targetAddress = server.IPaddress.getCanonicalHostName();
            	} else {
            		targetAddress = server.hostName;
            	}
                journalService.println("Testing connectivity to SIP Server: " + targetAddress);
            	pinger = new Pinger(DiscoveryService.rand.nextInt(1234), targetAddress, 500);
            	if (pinger.ping()) {
            		results = NONE;
            	} else {
            		journalService.println("Failed to contact SIP Server: " + targetAddress);
            	}
            }
        }

        journalService.println("");
        return results;
    }

}
