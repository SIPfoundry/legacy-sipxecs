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
import org.sipfoundry.preflight.discovery.Device;
import org.sipfoundry.preflight.discovery.DiscoveryService;

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
        active = true;
        journalService.println("Starting device discovery...");

        LinkedList<Device> devices = ds.discover(localHostAddress, "255.255.255.0");

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
