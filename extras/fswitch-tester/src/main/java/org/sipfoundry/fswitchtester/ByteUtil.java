/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.fswitchtester;

import java.nio.ByteBuffer;

/**
 * Miscellaneous utility functions.
 * 
 * @author M. Ranganathan
 * 
 */
public class ByteUtil {

    /***************************************************************************
     * Constants
     **************************************************************************/

    /** The maximum number of bytes in a UDP packet. */
    public static final int MAX_UDP_PACKET_SIZE = 65537;

    /** Number of bytes in a Java short. */
    public static final int NUM_BYTES_IN_SHORT = 2;

    /** Number of bytes in a Java int. */
    public static final int NUM_BYTES_IN_INT = 4;

    /** Number of bytes in a Java long. */
    public static final int NUM_BYTES_IN_LONG = 8;

    private static long maxValueCache[] = new long[64];

    static {
        for (int i = 1; i < 64; i++) {
            maxValueCache[i] = (long) (((long) 1 << i) - 1);
        }
    }

    /***************************************************************************
     * Methods
     **************************************************************************/

    /**
     * Convert a Java short to a 2-byte array.
     * 
     * @param s
     *            A short value.
     * @return A 2-byte array representing the short value.
     */
    public static byte[] shortToBytes(short s) {

        ByteBuffer byteBuffer = ByteBuffer.allocate(NUM_BYTES_IN_SHORT);
        byteBuffer.putShort(s);
        return byteBuffer.array();

    }

    /**
     * Convert a byte array to a Java short.
     * 
     * @param bytes
     *            A byte array.
     * @return A Java short.
     */
    public static short bytesToShort(byte[] bytes) {

        ByteBuffer byteBuffer = ByteBuffer.wrap(bytes);
        return byteBuffer.getShort();

    }

    /**
     * Convert a Java int to a 4-byte array.
     * 
     * @param i
     *            A Java int value.
     * @return A 4-byte array representing the int value.
     */
    public static byte[] intToBytes(int i, byte[] backingStore, int offset) {

        ByteBuffer byteBuffer = ByteBuffer.allocate(NUM_BYTES_IN_INT);
        byteBuffer.putInt(i);
        return byteBuffer.array();

    }

    /**
     * Convert a byte array to a Java int.
     * 
     * @param bytes
     *            A byte array.
     * @return A Java int.
     */
    public static int bytesToInt(byte[] bytes) {

        ByteBuffer byteBuffer = ByteBuffer.wrap(bytes);
        return byteBuffer.getInt();

    }

    /**
     * Convert an int to a hex representation.
     * 
     * @param s
     *            A Java int.
     * @return A hex representation of the int.
     */
    public static String intToHex(int i) {

        return Integer.toHexString(i);

    }

    /**
     * Convert a Java long to a 4-byte array.
     * 
     * @param l
     *            A Java long value.
     * @return A 4-byte array representing the int value.
     */
    public static byte[] longToBytes(long l) {

        ByteBuffer byteBuffer = ByteBuffer.allocate(NUM_BYTES_IN_LONG);
        byteBuffer.putLong(l);
        return byteBuffer.array();

    }

    /**
     * Convert a byte array to a Java long.
     * 
     * @param bytes
     *            A byte array.
     * @return A Java long.
     */
    public static long bytesToLong(byte[] bytes) {

        ByteBuffer byteBuffer = ByteBuffer.wrap(bytes);
        return byteBuffer.getLong();

    }

    /**
     * Convert a long to a hex representation.
     * 
     * @param s
     *            A Java long.
     * @return A hex representation of the long.
     */
    public static String longToHex(long l) {

        return Long.toHexString(l);

    }

    /**
     * Convert a hex representation to a Java long.
     * 
     * @param s
     *            A hex representation.
     * @return A Java long.
     */
    public static long hexToLong(String s) {

        return Long.parseLong(s, 16);

    }

    /**
     * Get a byte array in a printable binary form.
     * 
     * @param bytes
     *            The byte to be writen.
     * @return A String reprentation of the byte.
     */
    public static String writeBytes(byte[] bytes) {
        try {
            StringBuffer stringBuffer = new StringBuffer();

            for (int i = 0; i < bytes.length; i++) {
                stringBuffer.append(writeBits(bytes[i]));
                if (i != bytes.length)
                    stringBuffer.append(" ");
            }

            return stringBuffer.toString();
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

    }

    /**
     * Get a byte array in a printable binary form.
     * 
     * @param bytes
     *            The byte to be writen.
     * @return A String reprentation of the byte.
     */
    public static String writeBytes(byte[] bytes, int packetLength) {

        StringBuffer stringBuffer = new StringBuffer();

        for (int i = 0; i < packetLength; i++) {

            stringBuffer.append(writeBits(bytes[i]) + " ");

        }

        return stringBuffer.toString();

    }

    /**
     * Get a byte in a printable binary form.
     * 
     * @param b
     *            The byte to be writen.
     * @return A String reprentation of the byte.
     */
    private static String writeBits(byte b) {

        int lsByte = (int) (Byte.valueOf(b) & 0x0f);
        int msByte = (int) ((Byte.valueOf(b) >> 4) & 0x0f);
        return Integer.toHexString(msByte) + Integer.toHexString(lsByte);

    }

    /**
     * Get the maximum value for the number of unsigned bits.
     * 
     * @param i
     *            The number of unsigned bits.
     * @return The maximum value for the number of unsigned bits.
     */
    public static int getMaxIntValueForNumBits(int i) {

        if (i >= 32)
            throw new RuntimeException("Number of bits exceeds Java int.");
        else
            return (int) maxValueCache[i];

    }

    /**
     * Get the maximum value for the number of unsigned bits.
     * 
     * @param i
     *            The number of unsigned bits.
     * @return The maximum value for the number of unsigned bits.
     */
    public static long getMaxLongValueForNumBits(int i) {

        if (i >= 64)
            throw new RuntimeException("Number of bits exceeds Java long.");
        else
            return maxValueCache[i];

    }

    public static byte[] readBytes(String value) {

        String[] byteStrings = value.split(" ");

        byte[] retval = new byte[byteStrings.length];

        int i = 0;
        for (String byteString : byteStrings) {
            Byte decoded = Byte.decode("0x" + byteString);
            retval[i++] = decoded.byteValue();

        }
        return retval;
    }

}
