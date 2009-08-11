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
public class MessageSizeOption extends DHCPOption {
    private int messageSize;

    public MessageSizeOption() {
        super.setCode(MESSAGE_SIZE);
    }

    public MessageSizeOption(int messageSize) {
        super.setCode(MESSAGE_SIZE);
        super.setLength(2);
        this.messageSize = messageSize;
    }

    public long getMessageSize() {
        return messageSize;
    }

    public void setMessageSize(int messageSize) {
        this.messageSize = messageSize;
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        try {
            dataStream.writeByte(super.getCode().toInt());
            dataStream.writeByte(super.getLength());
            dataStream.writeShort(messageSize);
        } catch (IOException e) {
            System.err.println(e);
        }

        return byteStream.toByteArray();
    }

    public void unmarshal(DataInputStream dataStream) throws IOException {
        int length = dataStream.readByte() & 0xFF;
        if (length != 2) {
            throw new IOException();
        }
        super.setLength(length);
        messageSize = dataStream.readShort();
    }

}
