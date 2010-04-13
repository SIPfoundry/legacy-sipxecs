/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;

/**
 * Build payload of SIP NOTIFY restart message for grandstream phones
 */
class ResetPacket {
    private static final int MILLIS_IN_SECOND = 1000;
    private static final int MESSAGE_LEN = 24;
    private byte[] m_message;

    ResetPacket(String password, String macAddress) {
        m_message = new byte[MESSAGE_LEN];
        MessageDigest mymd;

        try {
            mymd = MessageDigest.getInstance("MD5");
        } catch (NoSuchAlgorithmException blaa) {
            throw new IllegalArgumentException("MD5 unknown");
        }

        m_message[0] = 0;
        m_message[1] = 1;
        m_message[2] = 0;
        m_message[3] = 0;

        int timeCode = getTimeCode();
        m_message[4] = (byte) ((timeCode >> 24) & 0xff);
        m_message[5] = (byte) ((timeCode >> 16) & 0xff);
        m_message[6] = (byte) ((timeCode >> 8) & 0xff);
        m_message[7] = (byte) (timeCode & 0xff);

        mymd.reset();
        try {
            mymd.update(Hex.decodeHex(macAddress.toCharArray()));
        } catch (DecoderException e) {
            throw new RuntimeException("hex string must be multiples of 2", e);
        }
        byte[] colon = new byte[] {
            ':'
        };
        mymd.update(colon);
        mymd.update(password.getBytes());
        mymd.update(colon);
        mymd.update(m_message, 4, 4);
        System.arraycopy(mymd.digest(), 0, m_message, 8, 16);
    }

    protected int getTimeCode() {
        Date time = Calendar.getInstance(TimeZone.getTimeZone("GMT")).getTime();
        int timeCode = (int) (time.getTime() / MILLIS_IN_SECOND);
        return timeCode;
    }

    byte[] getResetMessage() {
        return m_message;
    }
}
