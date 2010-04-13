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
public class BootFileOption extends DHCPOption {
    private String bootFile;

    public BootFileOption() {
        super.setCode(BOOT_FILE);
    }

    public BootFileOption(String bootFile) {
        super.setCode(BOOT_FILE);
        super.setLength(bootFile.length());
        this.bootFile = bootFile;
    }

    public String getBootFile() {
        return bootFile;
    }

    public void setBootFile(String bootFile) {
        this.bootFile = bootFile;
        super.setLength(bootFile.length());
    }

    public String toString() {
        return bootFile;
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        try {
            dataStream.writeByte(super.getCode().toInt());
            dataStream.writeByte(super.getLength());
            dataStream.write(bootFile.getBytes("ISO-8859-1"));
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
        bootFile = new String(stringBuffer, 0, length, "ISO-8859-1").trim();
    }

}
