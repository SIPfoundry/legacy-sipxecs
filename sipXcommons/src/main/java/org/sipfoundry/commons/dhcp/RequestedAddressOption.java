/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dhcp;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.InetAddress;

import static org.sipfoundry.commons.dhcp.DHCPOption.Code.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class RequestedAddressOption extends DHCPOption {
    private InetAddress requestedAddress;

    public RequestedAddressOption() {
        super.setCode(REQUESTED_ADDRESS);
    }

    public RequestedAddressOption(InetAddress requestedAddress) {
        super.setCode(REQUESTED_ADDRESS);
        super.setLength(4);
        this.requestedAddress = requestedAddress;
    }

    public InetAddress getRequestedAddress() {
        return requestedAddress;
    }

    public void setRequestedAddress(InetAddress requestedAddress) {
        this.requestedAddress = requestedAddress;
    }

    public String toString() {
        return requestedAddress.getHostAddress();
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        try {
            dataStream.writeByte(super.getCode().toInt());
            dataStream.writeByte(super.getLength());
            dataStream.write(requestedAddress.getAddress(), 0, 4);
        } catch (IOException e) {
            System.err.println(e);
        }

        return byteStream.toByteArray();
    }

    public void unmarshal(DataInputStream dataStream) throws IOException {
        int length = dataStream.readByte() & 0xFF;
        if (length != 4) {
            throw new IOException();
        }
        super.setLength(length);
        byte[] addressBuffer = new byte[4];
        dataStream.readFully(addressBuffer, 0, 4);
        requestedAddress = InetAddress.getByAddress(addressBuffer);
    }

}
