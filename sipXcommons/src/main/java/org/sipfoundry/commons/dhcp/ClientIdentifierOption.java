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
import java.util.StringTokenizer;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ClientIdentifierOption extends DHCPOption {
    private HardwareAddressType hardwareAddressType;
    private byte[] clientIdentifier;
    private int clientIdentifierLength;

    public ClientIdentifierOption() {
        super.setCode(CLIENT_IDENTIFIER);
    }

    public ClientIdentifierOption(HardwareAddressType hardwareAddressType, String clientIdentifierString) {
        super.setCode(CLIENT_IDENTIFIER);
        this.hardwareAddressType = hardwareAddressType;
        StringTokenizer tokenizer = new StringTokenizer(clientIdentifierString, ":");
        clientIdentifierLength = tokenizer.countTokens();
        super.setLength(clientIdentifierLength + 1);
        clientIdentifier = new byte[clientIdentifierLength];
        for (int x = 0; x < clientIdentifierLength; x++) {
            clientIdentifier[x] = (byte) Integer.parseInt(tokenizer.nextToken(), 16);
        }
    }

    public byte[] getClientIdentifier() {
        return clientIdentifier;
    }

    public void setClientIdentifier(HardwareAddressType hardwareAddressType, String clientIdentifierString) {
        this.hardwareAddressType = hardwareAddressType;
        StringTokenizer tokenizer = new StringTokenizer(clientIdentifierString, ":");
        clientIdentifierLength = tokenizer.countTokens();
        super.setLength(clientIdentifierLength + 1);
        clientIdentifier = new byte[clientIdentifierLength];
        for (int x = 0; x < clientIdentifierLength; x++) {
            clientIdentifier[x] = (byte) Integer.parseInt(tokenizer.nextToken(), 16);
        }
    }

    public String toString() {
        return "TBD";
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        try {
            dataStream.writeByte(super.getCode().toInt());
            dataStream.writeByte(super.getLength());
            dataStream.writeByte(hardwareAddressType.toInt());
            dataStream.write(clientIdentifier, 0, clientIdentifierLength);
        } catch (IOException e) {
            System.err.println(e);
        }

        return byteStream.toByteArray();
    }

    public void unmarshal(DataInputStream dataStream) throws IOException {
        int length = dataStream.readByte() & 0xFF;
        if (length < 2) {
            throw new IOException();
        }
        super.setLength(length);
        hardwareAddressType = HardwareAddressType.toEnum(dataStream.readByte() & 0xFF);
        clientIdentifierLength = length - 1;
        clientIdentifier = new byte[clientIdentifierLength];
        dataStream.readFully(clientIdentifier, 0, clientIdentifierLength);
    }

}
