/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.URL;
import java.util.Random;
import java.util.concurrent.DelayQueue;

import javax.sound.sampled.*;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;

/**
 * @author Woof!
 * @author mranga
 */

public class AudioFileStreamer {
    private static Logger logger = Logger.getLogger(AudioFileStreamer.class);
    static {
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
    }

    AudioInputStream ulawStream = null; // A local uLaw stream of bytes to packetize
    DatagramPacket rtpOutPacket;
    byte[] rtpOutBuf;
    private int rhythm = 20;
    private int payloadSize = 8*rhythm;
    long timestamp = 0L;
    long sequenceNumber; // The last RTP sequence number sent
    private DatagramSocket datagramSocket;
    private byte[] ssrcArray = new byte[4];
    private long timeOfDay;
   

    class RtpSender implements Runnable {

        RtpSender() {

        }

        public void run() {
            try {
                while (getLocalAudioRtp()) {
                    rtpSend();
                    Thread.sleep(rhythm);
                }
                System.out.println("Done");
            } catch (Exception ex) {
                ex.printStackTrace();
            }

        }

    }

    void rtpSend() throws IOException {
        byte[] octets = rtpOutPacket.getData();

        // dumpBytes(octets, 12) ;
        sequenceNumber = ++sequenceNumber & 0xFFFF; // Increment and wrap at 16 bits

        // Install sequence number into RTP header in big endian order
        octets[2] = (byte) ((sequenceNumber & 0xFF00) >> 8);
        octets[3] = (byte) ((sequenceNumber & 0x00FF));

        // Calculate payload length
        int numSamples = rtpOutPacket.getLength() - 12; // 12 bytes of header, minimum
        /*
         * if ((octets[0] & 0x20) != 0) {
         * 
         * int paddingSize = octets[rtpOutPacket.getLength() - 1]; numSamples -= paddingSize; }
         * int csrcCount = octets[0] & 0x0F; numSamples -= csrcCount * 4;
         */

        // Increment the timestamp by the payload length
        // (assumes one byte per sample codec, like PCMU)
        timestamp = (timestamp + numSamples) & 0xFFFFFFFF; // Increment and wrap at 32 bits

        // Install into RTP header in big endian order
        octets[4] = (byte) ((timestamp & 0xFF000000) >> 24);
        octets[5] = (byte) ((timestamp & 0x00FF0000) >> 16);
        octets[6] = (byte) ((timestamp & 0x0000FF00) >> 8);
        octets[7] = (byte) ((timestamp & 0x000000FF));

        // Install ssrc into RTP header in big endian order
        octets[8] = ssrcArray[0];
        octets[9] = ssrcArray[1];
        octets[10] = ssrcArray[2];
        octets[11] = ssrcArray[3];

        // if (debug)
        // dumpBytes(octets, origRtp.getLength()) ;

        datagramSocket.send(this.rtpOutPacket);
    }

    public AudioFileStreamer(String file, long startSequenceNumber, DatagramSocket datagramSocket, String ipAddr, int port)
            throws Exception {
        payloadSize = rhythm * 8; // 8 bytes per mS at 8000 samples/second uLaw
        rtpOutBuf = new byte[12 + payloadSize]; // 12 byte RTP header + payload
        rtpOutPacket = new DatagramPacket(rtpOutBuf, rtpOutBuf.length);
        this.datagramSocket = datagramSocket;
        URL fileUrl = new URL("file:" + file);
        this.startLocalAudio(fileUrl);
        this.sequenceNumber = startSequenceNumber;
        new Random().nextBytes(ssrcArray);

        rtpOutPacket.setSocketAddress(new InetSocketAddress(ipAddr, port));

    }

    public void send() {
        new Thread(new RtpSender()).start();
    }

    public long sendSynchronous() {
        try {
            int i = 0;
            this.timeOfDay = System.currentTimeMillis();
            
            while (getLocalAudioRtp()) {
                rtpSend();
                i++;
                long wakeupTime = timeOfDay + i * this.rhythm;
                
                int timeToSleep = ( int )  (  wakeupTime - System.currentTimeMillis()) ;
                if ( timeToSleep > 0 ) {
                    Thread.sleep(timeToSleep);
                }
            }
            System.out.println("Done");
            return this.sequenceNumber;
        } catch (Exception ex) {
            ex.printStackTrace();
            logger.error("Done sending file ");
            return -1;
        }
    }

    public void startLocalAudio(URL audioSource) {
        final AudioFormat ULAW_FORMAT_1 = new AudioFormat(AudioFormat.Encoding.ULAW, 8000f, // sample
                // rate
                8, // bits per sample
                1, // channels
                1, // frame rate
                8000f, // frame size
                false); // isBigEndian

        ulawStream = null;
        String reason = "Unable to open " + audioSource.toString();
        try {
            AudioInputStream inStream = AudioSystem.getAudioInputStream(audioSource);
            if (inStream.getFormat() == ULAW_FORMAT_1) {
                ulawStream = inStream;
            } else {
                reason = "Unable to convert " + inStream.toString() + " to uLaw.";
                // Convert steam to uLaw if possible
                ulawStream = AudioSystem.getAudioInputStream(ULAW_FORMAT_1, inStream);
                reason = "success";
            }
        } catch (Exception ex) {
            ex.printStackTrace();
            logger.error("Unexpected exception " + reason, ex);
        }

    }

    /**
     * Get the next RTP packet of local audio.
     * 
     * @return A datagram of an RTP packet containing the audio payload, or null if none is
     *         available.
     */
    boolean getLocalAudioRtp() {
        if (ulawStream == null) {
            // No stream is available, return null ;
            return false;
        }

        byte[] octets = rtpOutPacket.getData();
        int n = 0;
        try {
            // Read bytes from the local audio stream into the payload section of the RTP packet
            n = ulawStream.read(octets, 12, payloadSize);
            if (n > 0) {
                if (n < payloadSize) {
                    for (; n < payloadSize; n++) {
                        octets[n + 12] = (byte) 0xff; // Pad rest of payload with with uLaw
                        // silence
                    }
                }

                // Fill in static RTP header bytes
                octets[0] = (byte) 0x80; // V=2, CC=0
                octets[1] = (byte) 0; // M=0, PT=0 (uLaw)

                return true;
            }
            logger.debug("RtpFork::getLocalAudioRtp EOF on ulawStream");

        } catch (IOException e) {
            logger.error("RtpFork::getLocalAudioRtp", e);
        }

        // When an error occurs or EOF is reached, close the stream
        stopLocalAudio();
        return true;
    }

    public void stopLocalAudio() {
        logger.debug(String.format("RtpFork::closeLocalAudio"));
        try {
            if (ulawStream != null) {
                ulawStream.close();
                ulawStream = null;
            }
        } catch (Exception e) {
        }

    }

}
