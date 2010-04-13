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
public class TFTPServerOption extends DHCPOption {
    private String serverName;

    public TFTPServerOption() {
        super.setCode(TFTP_SERVER);
    }

    public TFTPServerOption(String serverName) {
        super.setCode(TFTP_SERVER);
        super.setLength(serverName.length());
        this.serverName = serverName;
    }

    public String getServerName() {
        return serverName;
    }

    public void setServerName(String serverName) {
        this.serverName = serverName;
        super.setLength(serverName.length());
    }

    public String toString() {
        return serverName;
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        try {
            dataStream.writeByte(super.getCode().toInt());
            dataStream.writeByte(super.getLength());
            dataStream.write(serverName.getBytes("ISO-8859-1"));
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
        serverName = new String(stringBuffer, 0, length, "ISO-8859-1").trim();
    }

}
