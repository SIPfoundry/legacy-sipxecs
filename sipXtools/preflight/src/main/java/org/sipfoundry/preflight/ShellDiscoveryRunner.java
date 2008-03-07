/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.LinkedList;

import org.eclipse.swt.widgets.*;
import static org.sipfoundry.preflight.ResultCode.*;
import org.sipfoundry.commons.discovery.Device;
import org.sipfoundry.commons.discovery.DiscoveryService;

/**
 * [Enter descriptive text here]
 * <p>
 * 
 * @author Mardy Marshall
 */
public class ShellDiscoveryRunner {
    private final Display display;
    private final JournalService journalService;
    private final DiscoveryService ds;
    private String localHostAddress = "0.0.0.0";

    private boolean active;

    ShellDiscoveryRunner(Display display, JournalService journalService) {
        this.display = display;
        this.journalService = journalService;

        InetAddress inetAddress;

        try {
            // Get local IP Address
            inetAddress = InetAddress.getLocalHost();
            localHostAddress = inetAddress.getHostAddress();
        } catch (UnknownHostException e1) {
            // Ignore.
        }

        ds = new DiscoveryService(localHostAddress, 5050, journalService);
    }

    public void discover() {
        DHCP dhcp = new DHCP();
        active = true;
        journalService.println("Starting device discovery...");

        ResultCode results;
        NetworkResources networkResources = new NetworkResources();
        InetAddress bindAddress;
        try {
            bindAddress = InetAddress.getByName("0.0.0.0");
        } catch (UnknownHostException e) {
            e.printStackTrace();
            return;
        }

        results = dhcp.validate(10, networkResources, journalService, bindAddress);
        if (results == NONE) {
            String networkAddress = null;
            String networkMask = null;
            InetAddress hostAddress = null;

            try {
                // Get local IP Address
                hostAddress = InetAddress.getLocalHost();
                localHostAddress = hostAddress.getHostAddress();
            } catch (UnknownHostException e1) {
                // Ignore.
            }

            // Calculate the network address.
            byte[] rawMaskAddress;
            byte[] rawHostAddress;
            byte[] rawNetworkAddress = new byte[4];
            rawMaskAddress = networkResources.subnetMask.getAddress();
            rawHostAddress = hostAddress.getAddress();
            for (int i = 3; i >= 0; i--) {
                rawNetworkAddress[i] = (byte) (rawHostAddress[i] & rawMaskAddress[i]);
            }
            try {
                networkAddress = InetAddress.getByAddress(rawNetworkAddress).getHostAddress();
                networkMask = InetAddress.getByAddress(rawMaskAddress).getHostAddress();
            } catch (UnknownHostException e) {
                // Ignore.
            }

            LinkedList<Device> devices = ds.discover(networkAddress, networkMask, false);

            if (devices.size() == 0) {
                journalService.println("No devices discovered.");
            } else {
                journalService.println("Discovery completed.");
            }
        } else {
            journalService.println("Discovery failed due to DHCP error.");
        }

        active = false;
    }

    public boolean isActive() {
        return active;
    }
}
