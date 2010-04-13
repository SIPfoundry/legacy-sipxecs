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
public class MessageTypeOption extends DHCPOption {
    public enum Type {
        DHCPDISCOVER(1),
        DHCPOFFER(2),
        DHCPREQUEST(3),
        DHCPDECLINE(4),
        DHCPACK(5),
        DHCPNAK(6),
        DHCPRELEASE(7),
        DHCPINFORM(8),
        INVALID(-1);

        /**
         * Storage for integer representation of enumeration.
         */
        private int integerValue;

        /**
         * Default constructor.
         *
         * @param integerValue
         *            Integer representation of enumeration.
         */
        Type(int integerValue) {
            this.integerValue = integerValue;
        }

        /**
         * Retrieve the integer representation of this enumeration.
         *
         * @return Integer representation of this enumeration.
         */
        public int toInt() {
            return integerValue;
        }

        /**
         * Map the given integer to a corresponding ENUM value.
         *
         * @param integerValue
         *            to convert.
         * @return Corresponding ENUM value. If no match, returns INVALID.
         */
        public static Type toEnum(int integerValue) {
            switch (integerValue) {
                case 1:
                    return DHCPDISCOVER;
                case 2:
                    return DHCPOFFER;
                case 3:
                    return DHCPREQUEST;
                case 4:
                    return DHCPDECLINE;
                case 5:
                    return DHCPACK;
                case 6:
                    return DHCPNAK;
                case 7:
                    return DHCPRELEASE;
                case 8:
                    return DHCPINFORM;
                default:
                    return INVALID;
            }
        }
    }

    private Type messageType;

    public MessageTypeOption() {
        super.setCode(DHCP_MESSAGE_TYPE);
    }

    public MessageTypeOption(Type messageType) {
        super.setCode(DHCP_MESSAGE_TYPE);
        super.setLength(1);
        this.messageType = messageType;
    }

    public Type getMessageType() {
        return messageType;
    }

    public void setMessageType(Type messageType) {
        this.messageType = messageType;
    }

    public String toString() {
        switch (messageType) {
            case DHCPDISCOVER:
                return "DHCPDISCOVER";
            case DHCPOFFER:
                return "DHCPOFFER";
            case DHCPREQUEST:
                return "DHCPREQUEST";
            case DHCPDECLINE:
                return "DHCPDECLINE";
            case DHCPACK:
                return "DHCPACK";
            case DHCPNAK:
                return "DHCPNAK";
            case DHCPRELEASE:
                return "DHCPRELEASE";
            case DHCPINFORM:
                return "DHCPINFORM";
            default:
                return "";
        }
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        try {
            dataStream.writeByte(super.getCode().toInt());
            dataStream.writeByte(super.getLength());
            dataStream.writeByte(messageType.toInt());
        } catch (IOException e) {
            System.err.println(e);
        }

        return byteStream.toByteArray();
    }

    public void unmarshal(DataInputStream dataStream) throws IOException {
        int length = dataStream.readByte() & 0xFF;
        if (length != 1) {
            throw new IOException();
        }
        super.setLength(length);
        messageType = Type.toEnum(dataStream.readByte() & 0xFF);
    }

}
