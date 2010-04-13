/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.discovery;

import java.net.InetAddress;
import java.net.UnknownHostException;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class Device {
    private InetAddress networkAddress;
    private final byte[] hardwareAddress;
    private final String vendor;
    private final String userAgentInfo;

    Device(String hardwareAddress, String networkAddress, String vendor, String userAgentInfo) {
        this.hardwareAddress = new byte[6];
        String[] fields = hardwareAddress.split("[:,-]");
        if (fields.length == 6) {
            for (int i = 0; i < 6; i++) {
                Integer x = Integer.parseInt(fields[i], 16) & 0xFF;
                this.hardwareAddress[i] = x.byteValue();
            }
        } else {
            for (int i = 0; i < 6; i++) {
                this.hardwareAddress[i] = 0;
            }
        }

        try {
            this.networkAddress = InetAddress.getByName(networkAddress);
        } catch (UnknownHostException e) {
            // Just eat the exception.
        }

        this.vendor = vendor;
        this.userAgentInfo = userAgentInfo;
    }

    public String getHardwareAddress() {
        String address;
        address = String.format("%02X:%02X:%02X:%02X:%02X:%02X",
                hardwareAddress[0],
                hardwareAddress[1],
                hardwareAddress[2],
                hardwareAddress[3],
                hardwareAddress[4],
                hardwareAddress[5]);
        return address;
    }

    public InetAddress getNetworkAddress() {
        return networkAddress;
    }

    public String getVendor() {
        return vendor;
    }

    public String getUserAgentInfo() {
        return userAgentInfo;
    }
}
