/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dhcp;

import static org.sipfoundry.commons.dhcp.DHCPOption.Code.*;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class VendorIdentifierOption extends DHCPOption {
    private String vendorIdentifier;

    public VendorIdentifierOption() {
        super.setCode(VENDOR_IDENTIFIER);
    }

    public VendorIdentifierOption(String vendorIdentifier) {
        super.setCode(VENDOR_IDENTIFIER);
        super.setLength(vendorIdentifier.length());
        this.vendorIdentifier = vendorIdentifier;
    }

    public String getVendorIdentifier() {
        return vendorIdentifier;
    }

    public void setVendorIdentifier(String vendorIdentifier) {
        this.vendorIdentifier = vendorIdentifier;
        super.setLength(vendorIdentifier.length());
    }

    public String toString() {
        return vendorIdentifier;
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        try {
            dataStream.writeByte(super.getCode().toInt());
            dataStream.writeByte(super.getLength());
            dataStream.write(vendorIdentifier.getBytes("ISO-8859-1"));
        } catch (IOException e) {
            System.err.println(e);
        }

        return byteStream.toByteArray();
    }

    public void unmarshal(DataInputStream dataStream) throws IOException {
        int length = dataStream.readByte() & 0xFF;
        if (length < 1) {
            throw new IOException();
        }
        super.setLength(length);
        byte[] stringBuffer = new byte[length];
        dataStream.readFully(stringBuffer, 0, length);
        vendorIdentifier = new String(stringBuffer, 0, length, "ISO-8859-1").trim();
    }

}
