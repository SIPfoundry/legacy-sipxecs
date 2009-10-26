/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.nio.ByteBuffer;
import java.util.Random;

/**
 * A simple RTP packet class.
 * 
 * @author M. Ranganathan
 * 
 */

class DummyRtpPacket {
    private static int seqCounter;
    /*********************************************************************************************
     * Constants
     ********************************************************************************************/

    /** Constant that identifies the total byte length of fixed fields. */
    public final static int FIXED_HEADER_LENGTH = 12; // V..SSRC only

    /** The maximum buffer (byte array) size for payload data. */
    public static final int MAX_PAYLOAD_BUFFER_SIZE = 512;

    /*********************************************************************************************
     * Variables
     ********************************************************************************************/

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

    private static ByteBuffer byteBuffer;

    /** Contributing sources (32 bits) -- not supported yet. */
    // private CSRC;
    /** Header extension Defined By Profile (16 bits) -- not supported yet. */
    // private short DP = 0;
    /** Header extension length (16 bits) -- not supported yet. */
    // private short EL = 0;
    /**
     * The payload. Transient because we are only interested in the headers for comparing.
     */
    private transient byte[] payload = null;

    /** The length of the payload. */
    private int payloadLength = 0;

    private static long myssrc = Math.abs(new Random().nextLong());

    /*********************************************************************************************
     * Constructor
     ********************************************************************************************/

    /**
     * Construct an RTP packet. No byte buffer for this one. This constructs an empty packet.
     */
    public DummyRtpPacket() {

    }

    /*********************************************************************************************
     * Methods
     ********************************************************************************************/

    /**
     * Get the data of this RTP packet as a byte array.
     * 
     * @return The data of this RTP packet as a byte array.
     */
    private ByteBuffer getData() {

        ByteBuffer byteBuffer = ByteBuffer.allocate(12 + payloadLength);

        /* Since V..SN are 32 bits, create a (int) byte array for V..SN. */
        long V_SN = 0;
        V_SN = ((long) version) << 30 | this.padding << 29 | extension << 28 | csrcCount << 24
                | this.marker << 23 | this.payloadType << 16 | (sequenceNumber & 0xffff);

        byteBuffer.putInt((int) V_SN);

        // offset = 4 from the start of packet
        byteBuffer.putInt((int) this.timeStamp);

        // offset = 8 from start of packet
        byteBuffer.putInt((int) this.SSRC);

        if (payloadLength != 0) {
            // This only applies if somebody has tinkered with the payload.
            // offset = 12 from start of packet.
            byteBuffer.put(payload);
        }
        // Reset pointer to start of buffer.
        byteBuffer.rewind();

        return byteBuffer;

    }

    /**
     * Set the payload of this RTP packet.
     * 
     * @param bytes the byte buffer containing the payload
     * @param length the number of buffer bytes containing the payload.
     */
    private void setPayload(byte[] bytes, int length) throws IllegalArgumentException {

        if (length > MAX_PAYLOAD_BUFFER_SIZE)
            throw new IllegalArgumentException("Payload is too large Max Size is limited to "
                    + MAX_PAYLOAD_BUFFER_SIZE);

        payloadLength = length;
        payload = bytes;

    }

    public static ByteBuffer createDummyRtpPacket() {

        DummyRtpPacket rtpPacket = new DummyRtpPacket();
        rtpPacket.payloadType = 0;
        byte[] payload = new byte[128];
        for (int i = 0; i < payload.length; i++) {
            payload[i] = (byte) 0xff;
        }
        rtpPacket.setPayload(payload, payload.length);
        rtpPacket.sequenceNumber = 12345 + seqCounter++;
        rtpPacket.SSRC = new Random().nextLong();
        byteBuffer = rtpPacket.getData();

        return byteBuffer;

    }

}
