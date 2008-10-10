package org.sipfoundry.fswitchtester;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.DatagramChannel;
import java.nio.channels.Selector;
import java.util.Random;
import java.util.concurrent.DelayQueue;
import java.util.concurrent.Delayed;
import java.util.concurrent.TimeUnit;

import jpcap.JpcapCaptor;
import jpcap.packet.Packet;
import jpcap.packet.UDPPacket;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;

public class PcapRtpStream {
    private long startTime = -1;
    private static Logger logger = Logger.getLogger(PcapRtpStream.class);
    private DatagramChannel sendingChannel;
    private InetAddress origDestAddress;
    private int origDestPort;
    private JpcapCaptor captor;
    private java.util.concurrent.DelayQueue<DelayRecord> delayQueue = new DelayQueue<DelayRecord> ();
    private long startWallClockTime;
    private SocketAddress destinationSocketAddress;
    private InetAddress origSrcAddress;
    private int origSrcPort;
    private long ssrc;
    private int seqno;
    
    static {
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
    }

    class PacketSender implements Runnable {

        public void run() {
            try {
                while (readTraceFromFile()) {
                    DelayRecord delayRecord = delayQueue.take();
                    ByteBuffer byteBuffer = (ByteBuffer)delayRecord.rtpPacket.getData();
                
                    sendingChannel.send(byteBuffer,destinationSocketAddress);              
                }
                logger.error("Completed streaming the conversation");
            } catch ( ClosedChannelException e) {
                // Ignore.
            } catch (Exception e) {
                logger.error("Unepxected exception taking element from queue", e);
            }

        }

    }

    class DelayRecord implements Delayed {
        RtpPacket rtpPacket;
        private long timeToSend;

        public DelayRecord(RtpPacket rtpPacket, long timeToSend) {
            this.timeToSend = timeToSend;
            this.rtpPacket = rtpPacket;
        }

        public long getDelay(TimeUnit timeUnit) {
            if (timeToSend == 0) {
                return 0;
            } else {
                return timeUnit.convert(timeToSend - System.currentTimeMillis(), TimeUnit.MILLISECONDS);
            }

        }

        public int compareTo(Delayed delayed) {
            DelayRecord request = (DelayRecord) delayed;
            if (this.timeToSend < request.timeToSend) {
                return -1;
            } else if (this.timeToSend > request.timeToSend) {
                return 1;
            } else {
                return 0;
            }
        }

    }

    private void openFile(String fileName) {
        try {
            logger.debug("PacketMonitor : openFile : " + fileName);
            this.captor = JpcapCaptor.openFile(fileName);
            this.startWallClockTime = System.currentTimeMillis();
            this.ssrc = Math.abs( new Random().nextInt());
            this.seqno = 1;
        } catch (Exception ex) {
            throw new FreeSwitchTesterException("Exception opening captor file", ex);
        }
    }

    private boolean readTraceFromFile() {
        logger.debug("PacketMonitor : readTraceFromFile");
        Packet packet = captor.getPacket();
        while (packet != Packet.EOF) {

            if (packet instanceof jpcap.packet.UDPPacket) {
                logger.debug("got a packet ");
                UDPPacket udpPacket = (UDPPacket) packet;
                InetAddress dstIpAddress = udpPacket.dst_ip;
                int dstPort = udpPacket.dst_port;
                InetAddress srcIpAddress = udpPacket.src_ip;
                int srcPort = udpPacket.src_port;
                if (dstIpAddress.equals(this.origDestAddress) && dstPort == this.origDestPort
                    && srcIpAddress.equals(this.origSrcAddress) && srcPort == this.origSrcPort) {
                    byte[] data = udpPacket.data;
                    logger.debug("data.length = " + udpPacket.data.length);
                    RtpPacket rtpPacket = new RtpPacket((ByteBuffer)ByteBuffer.wrap(data).rewind());
                    logger.debug("ssrc = " + rtpPacket.getSSRC() + " seqno " + rtpPacket.getSequenceNumber() + " pt = " + rtpPacket.getPayloadType());
                    rtpPacket.setSSRC(ssrc);
                    rtpPacket.setSequenceNumber(this.seqno++);
                    

                    if (startTime == -1) {
                        startTime = packet.sec * 1000000 + packet.usec;
                        DelayRecord delayRecord = new DelayRecord(rtpPacket, 0);
                        this.delayQueue.add(delayRecord);
                        startWallClockTime = System.currentTimeMillis();
                    } else {
                        long delta = (packet.sec * 1000000 + packet.usec - startTime)/1000 ;
                        System.out.println("Delta = " + delta);
                        long timeToSend = delta + startWallClockTime;
                        DelayRecord delayRecord = new DelayRecord(rtpPacket, timeToSend);
                        this.delayQueue.add(delayRecord);
                    }
                    return true;
                } else {
                    packet = captor.getPacket();
                    continue;
                }

            } else {
                packet = captor.getPacket();
            }

        }
        return false;

    }

    public PcapRtpStream(String file, String originalSrcAddress, int origSrcPort,
            String originalDestAddress, int origDestPort,
            String sourceAddress, int sourcePort, String destAddress, int destPort) {

        try {
            
           

            this.origDestAddress = InetAddress.getByName(originalDestAddress);
            this.origDestPort = origDestPort;
            this.origSrcAddress = InetAddress.getByName(originalSrcAddress);
            this.origSrcPort = origSrcPort;
            InetAddress sourceInetAddress = InetAddress.getByName(sourceAddress);
         
            SocketAddress socketAddress = new InetSocketAddress(sourceInetAddress, sourcePort);
            this.sendingChannel = DatagramChannel.open();
            this.sendingChannel.socket().bind(socketAddress);
          
            //this.sendingChannel.bind(socketAddress);
            InetAddress destInetAddress = InetAddress.getByName(destAddress);
            this.destinationSocketAddress = new InetSocketAddress(destInetAddress, destPort);
           // this.sendingChannel.connect(destinationSocketAddress);
        } catch (Exception ex) {
            throw new FreeSwitchTesterException("Unexpected exception ", ex);
        }
        this.openFile(file);

    }
    
    public void send() {
        new Thread( new PacketSender() ).start();
    }
    
    public void close() throws IOException {
        captor.close();
        sendingChannel.close();
    }

}
