/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.LinkedList;

import org.eclipse.swt.widgets.*;

import org.sipfoundry.commons.dhcp.NetworkResources;
import org.sipfoundry.commons.discovery.Device;
import org.sipfoundry.commons.discovery.DiscoveryService;
import org.sipfoundry.commons.util.JournalService;

import static org.sipfoundry.preflight.ResultCode.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ShellDiscoveryRunner {
    private final JournalService journalService;
    private final DiscoveryService ds;
    private String localHostAddress = "0.0.0.0";

    private boolean active;

    ShellDiscoveryRunner(Display display, JournalService journalService) {
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

        journalService.disable();
        results = dhcp.validate(10, networkResources, journalService, bindAddress);
        journalService.enable();
        if (results != NONE) {
            // Unable to determine the network mask via DHCP.
            try {
                // Default to a Class C subnet.
                journalService.println("\nUnable to determine network mask via DHCP, defaulting to Class C.\n");
                networkResources.subnetMask = InetAddress.getByName("255.255.255.0");
            } catch (UnknownHostException e1) {
                // Ignore.
            }
        }

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

        active = false;
    }

    public boolean isActive() {
        return active;
    }
}
