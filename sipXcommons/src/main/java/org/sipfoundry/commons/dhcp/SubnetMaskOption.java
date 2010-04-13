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
public class SubnetMaskOption extends DHCPOption {
    private InetAddress mask;

    public SubnetMaskOption() {
        super.setCode(SUBNET_MASK);
    }

    public SubnetMaskOption(InetAddress mask) {
        super.setCode(SUBNET_MASK);
        super.setLength(4);
        this.mask = mask;
    }

    public InetAddress getMask() {
        return mask;
    }

    public void setMask(InetAddress mask) {
        this.mask = mask;
    }

    public String toString() {
        return mask.getHostAddress();
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        try {
            dataStream.writeByte(super.getCode().toInt());
            dataStream.writeByte(super.getLength());
            dataStream.write(mask.getAddress(), 0, 4);
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
        mask = InetAddress.getByAddress(addressBuffer);
    }

}
