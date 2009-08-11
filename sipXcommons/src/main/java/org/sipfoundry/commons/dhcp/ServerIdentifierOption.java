/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dhcp;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;

import static org.sipfoundry.commons.dhcp.DHCPOption.Code.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ServerIdentifierOption extends DHCPOption {
    private InetAddress serverIdentifier;

    public ServerIdentifierOption() {
        super.setCode(SERVER_IDENTIFIER);
    }

    public ServerIdentifierOption(InetAddress serverIdentifier) {
        super.setCode(SERVER_IDENTIFIER);
        super.setLength(4);
        this.serverIdentifier = serverIdentifier;
    }

    public InetAddress getServerIdentifier() {
        return serverIdentifier;
    }

    public void setServerIdentifier(InetAddress serverIdentifier) {
        this.serverIdentifier = serverIdentifier;
    }

    public String toString() {
        return serverIdentifier.getHostAddress();
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        try {
            dataStream.writeByte(super.getCode().toInt());
            dataStream.writeByte(super.getLength());
            dataStream.write(serverIdentifier.getAddress(), 0, 4);
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
        serverIdentifier = InetAddress.getByAddress(addressBuffer);
    }

}
