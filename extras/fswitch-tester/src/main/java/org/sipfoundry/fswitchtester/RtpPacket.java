/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.fswitchtester;

import java.nio.ByteBuffer;

import org.apache.log4j.Logger;

/**
 * A simple RTP packet class.
 * 
 * @author M. Ranganathan
 * 
 */
public class RtpPacket {
    /***************************************************************************
     * Constants
     **************************************************************************/

    /** Constant that identifies the total byte length of fixed fields. */
    public final static int FIXED_HEADER_LENGTH = 12; // V..SSRC only

    /** The maximum buffer (byte array) size for payload data. */
    public static final int MAX_PAYLOAD_BUFFER_SIZE = 512;

    /***************************************************************************
     * Variables
     **************************************************************************/

    private boolean dirty;

    /** Version number (2 bits). */
    private int version = 2;

    /** Padding (1 bit). */
    private int padding = 0;

    /** Header extension (1 bit). */
    private int extension = 0;

    /** CSRC count (4 bits). */
    private int csrcCount = 0;

    /** Marker (1 bit). */
    private int marker = 0;

    /** Payload type (7 bits). */
    private int payloadType = 0;

    /** Sequence number (16 bits). */
    private int sequenceNumber = 0;

    /** Time stamp (32 bits). */
    private long timeStamp = 0;

    /** Synchronization source (32 bits). */
    private long SSRC = 0;

    ByteBuffer byteBuffer;

    /** Contributing sources (32 bits) -- not supported yet. */
    // private CSRC;
    /** Header extension Defined By Profile (16 bits) -- not supported yet. */
    // private short DP = 0;
    /** Header extension length (16 bits) -- not supported yet. */
    // private short EL = 0;
    /**
     * The payload. Transient because we are only interested in the headers for
     * comparing.
     */
    private transient byte[] payload = null;

    /** The length of the payload. */
    private int payloadLength = 0;

    /** Scratchpad for unpacking */
    private byte[] bytes = new byte[8];

    /** The logger for this class. */
    private static Logger logger = Logger.getLogger(RtpPacket.class);

    private void unpack() {
        if (byteBuffer != null) {
            int V_SN = this.byteBuffer.getInt();
            version = (V_SN >>> 0x1E) & 0x03;
            padding = (V_SN >>> 0x1D) & 0x01;
            extension = (V_SN >>> 0x1C) & 0x01;
            csrcCount = (V_SN >>> 0x18) & 0x0F;
            marker = (V_SN >>> 0x17) & 0x01;
            payloadType = (V_SN >>> 0x10) & 0x7F;
            sequenceNumber = (V_SN & 0xFFFF);

            this.byteBuffer.get(bytes, 4, 4);
            timeStamp = ByteUtil.bytesToLong(bytes);

            this.byteBuffer.get(bytes, 4, 4);
            SSRC = ByteUtil.bytesToLong(bytes);

            if (version != 2)
                throw new RuntimeException("Unexpected value for version ! "
                        + version);
        }

    }

    /***************************************************************************
     * Constructor
     **************************************************************************/

    /**
     * Construct an RTP packet. No byte buffer for this one. This constructs an
     * empty packet.
     */
    public RtpPacket() {

    }

    /**
     * Set this RTP packet with the given ByteBuffer.
     * 
     * @param bytes
     *            The byte array for populating this RTP packet.
     * @param length
     *            The number of bytes to read from the byte array.
     */
    public RtpPacket(ByteBuffer bbuf) {
        this.byteBuffer = bbuf;
        //this.byteBuffer.flip();
        unpack();
        this.byteBuffer.rewind();

    }

    /***************************************************************************
     * Methods
     **************************************************************************/

    /**
     * Get the data of this RTP packet as a byte array.
     * 
     * @return The data of this RTP packet as a byte array.
     */
    public ByteBuffer getData() {
        if (this.byteBuffer == null) {
            // We need to pack every field in this case.
            // Header size is 12 bytes.
            this.byteBuffer = ByteBuffer.allocate(12 + payloadLength);
            this.dirty = true;

        }

        /* Since V..SN are 32 bits, create a (int) byte array for V..SN. */
        if (this.dirty) {
            long V_SN = 0;
            V_SN = ((long) version) << 30 | this.padding << 29
                    | extension << 28 | csrcCount << 24 | this.marker << 23
                    | this.payloadType << 16 | (sequenceNumber & 0xffff);

            logger.debug("Packing vsnBytes " + V_SN);
            this.byteBuffer.putInt((int) V_SN);

            // offset = 4 from the start of packet
            this.byteBuffer.putInt((int) this.timeStamp);

            // offset = 8 from start of packet
            this.byteBuffer.putInt((int) this.SSRC);

            if (payloadLength != 0) {
                // This only applies if somebody has tinkered with the payload.
                // offset = 12 from start of packet.
                this.byteBuffer.put(payload);
            }
            // Reset pointer to start of buffer.
            this.byteBuffer.rewind();

        }

        return byteBuffer;

    }

    /**
     * Get the RTP version.
     */
    public int getVersion() {

        return version;

    }

    /**
     * Set the padding bit.
     * 
     * @param i
     *            The padding (1 bit).
     * @throws IllegalArgumentException
     * @param i
     */
    public void setPadding(int i) throws IllegalArgumentException {

        this.dirty = true;

        if ((0 <= i) && (i <= ByteUtil.getMaxIntValueForNumBits(1)))
            padding = i;
        else
            throw new IllegalArgumentException("Out of range");

    }

    /**
     * Get the padding bit.
     * 
     * @return The padding.
     */
    public int getPadding() {

        return padding;

    }

    /**
     * Set the extension.
     * 
     * @param i
     *            The extension (1 bit)
     * @throws IllegalArgumentException
     */
    public void setExtension(int i) throws IllegalArgumentException {

        this.dirty = true;
        if ((0 <= i) && (i <= ByteUtil.getMaxIntValueForNumBits(1)))
            extension = i;
        else
            throw new IllegalArgumentException("Out of range");

    }

    /**
     * Get the extension.
     * 
     * @return the extension.
     */
    public int getExtension() {

        return extension;

    }

    /**
     * Set the CSRC count.
     * 
     * @param i
     *            The CSRC count (4 bits)
     * @throws IllegalArgumentException
     */
    public void setCsrcCount(int i) throws IllegalArgumentException {
        this.dirty = true;
        if ((0 <= i) && (i <= ByteUtil.getMaxIntValueForNumBits(4)))
            csrcCount = i;
        else
            throw new IllegalArgumentException("Out of range");

    }

    /**
     * Get the CSRC count.
     * 
     * @return the CSRC count.
     */
    public int getCsrcCount() {

        return csrcCount;

    }

    /**
     * Set the marker.
     * 
     * @param i
     *            The marker (1 bit)
     * @throws IllegalArgumentException
     */
    public void setMarker(int i) throws IllegalArgumentException {
        this.dirty = true;
        if ((0 <= i) && (i <= ByteUtil.getMaxIntValueForNumBits(1)))
            marker = i;
        else
            throw new IllegalArgumentException("Out of range");

    }

    /**
     * Get the marker.
     * 
     * @return the marker.
     */
    public int getMarker() {
        return marker;

    }

    /**
     * Set the payload type.
     * 
     * @param i
     *            The payload type (7 bits)
     * @throws IllegalArgumentException
     */
    public void setPayloadType(int i) throws IllegalArgumentException {
        this.dirty = true;
        if ((0 <= i) && (i <= ByteUtil.getMaxIntValueForNumBits(7)))
            payloadType = i;
        else
            throw new IllegalArgumentException("Out of range");

    }

    /**
     * Get the payload type.
     * 
     * @return The payload type.
     */
    public int getPayloadType() {

        return payloadType;

    }

    /**
     * Set the sequence number.
     * 
     * @param i
     *            The sequence number (16 bits)
     * @throws IllegalArgumentException
     */
    public void setSequenceNumber(int i) throws IllegalArgumentException {
        this.dirty = true;
        if ((0 <= i) && (i <= ByteUtil.getMaxIntValueForNumBits(16)))
            sequenceNumber = i;
        else
            throw new IllegalArgumentException("Out of range");

    }

    /**
     * Get the sequence number.
     * 
     * @return the sequence number.
     */
    public int getSequenceNumber() {

        return sequenceNumber;

    }

    /**
     * Set the time stamp.
     * 
     * @param timeStamp
     *            The time stamp (32 bits).
     * @throws IllegalArgumentException
     */
    public void setTimeStamp(long timeStamp) throws IllegalArgumentException {
        this.dirty = true;
        if ((0 <= timeStamp)
                && (timeStamp <= ByteUtil.getMaxLongValueForNumBits(32)))
            this.timeStamp = timeStamp;
        else
            throw new IllegalArgumentException("Out of range");

    }

    public long getTimeStamp() {

        return timeStamp;

    }

    /**
     * Set the synchronization source identifier.
     * 
     * @param ssrc
     *            the synchronization source identifier (32 bits)
     * @throws IllegalArgumentException
     */
    public void setSSRC(long ssrc) throws IllegalArgumentException {
        this.dirty = true;
        if ((0 < ssrc) && (ssrc <= ByteUtil.getMaxLongValueForNumBits(32)))
            SSRC = ssrc;
        else
            throw new IllegalArgumentException("Out of range" + ssrc);

    }

    /**
     * Get the synchronization source identifier.
     * 
     * @return the synchronization source identifier.
     */
    public long getSSRC() {

        return SSRC;

    }

    // public int getCSRC() {}
    // public void setCSRC() {}

    /***************************************************************************
     * RTP Header Extensions
     **************************************************************************/

    // public void getDP(){}
    // public void setDP(){}
    // public void getEL(){}
    // public void setEL(){}
    /***************************************************************************
     * Other Methods
     **************************************************************************/

    /**
     * Set the payload of this RTP packet.
     * 
     * @param bytes
     *            the byte buffer containing the payload
     * @param length
     *            the number of buffer bytes containing the payload.
     */
    public void setPayload(byte[] bytes, int length)
            throws IllegalArgumentException {

        if (logger.isDebugEnabled())
            logger.debug("Payload length: " + length);

        if (length > MAX_PAYLOAD_BUFFER_SIZE)
            throw new IllegalArgumentException(
                    "Payload is too large Max Size is limited to "
                            + MAX_PAYLOAD_BUFFER_SIZE);

        this.dirty = true;

        payloadLength = length;
        payload = bytes;

    }

    /**
     * Get the payload length.
     * 
     * @return they payload length.
     */
    public int getPayloadLength() {

        return payloadLength;

    }

}
