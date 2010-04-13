/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.LinkedList;
import java.util.Random;
import java.util.concurrent.atomic.AtomicInteger;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.ListeningPoint;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.header.CSeqHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class ConferenceTestClient implements SipListener {
    private static Logger logger = Logger.getLogger(ConferenceTestClient.class);
    private SipProvider provider;
    private int port;
    private String userName;
    private ListeningPoint listeningPoint;
    private String ipAddress;
    private String remoteIpAddress;
    private int remoteMediaPort;
    private boolean sdpReceived;
    private DatagramSocket datagramSocket;
    private int mediaPort;
    private Dialog dialog;
    private int testerId;
    private boolean isSender;
    private LinkedList<Double> powerList = new LinkedList<Double>();
    static LinkedList<Double> resultList = new LinkedList<Double>();
    static LinkedList<Double> jitterResultList = new LinkedList<Double>();
    private LinkedList<Long> jitterList = new LinkedList<Long>();
    private static AtomicInteger okCount = new AtomicInteger();
    private static final int TRIM = 30;
    private static final long createTime = System.currentTimeMillis();

    class MediaSender implements Runnable {
        private String fileName;

        public MediaSender(String fileName) throws Exception {
            this.fileName = fileName;

        }

        public void run() {

            try {

                synchronized (okCount) {
                    okCount.wait();
                }
                
                long sleepTime = ( FreeSwitchTester.getTesterConfig().getTesterClients().size() - FreeSwitchTester.getClientId() ) * 2500;
                
                
                long sequenceNumber = Math.abs(new Random().nextLong());

                AudioFileStreamer streamer = new AudioFileStreamer(fileName, sequenceNumber,
                        datagramSocket, remoteIpAddress, remoteMediaPort);

          
                      
                
                
                Thread.sleep( sleepTime );
               
                System.out.println("Sending stream client Id = " + testerId + " fileName = "
                        + fileName);
                streamer.sendSynchronous();
                
                Thread.sleep(5000);
                
                if (isComputeClient()) {

                    int count = 0;
                    double averagePower = 0;
                    int entries = 0;
                    for (Double power : powerList) {

                        if (count > TRIM - 1 && count < powerList.size() - TRIM) {
                            averagePower += power;
                            entries++;
                        }
                        count++;
                    }
                    
                    averagePower = averagePower / entries;
                    double stdDeviation = 0;
                    count = 0;
                    for (Double power : powerList) {
                        if (count > TRIM - 1 && count < powerList.size() - TRIM) {
                            stdDeviation = Math.pow(power  - averagePower, 2)
                                    + stdDeviation;
                            entries++;
                        }
                        count ++;
                        
                    }
                    
                    double fractionalRmsPowerDeviation = 0.0;
                    if (entries > 0 && averagePower > 0 ) {
                        double rmsPowerDeviation =(double)stdDeviation / (double)entries;
                        fractionalRmsPowerDeviation = rmsPowerDeviation / averagePower;
                        synchronized (resultList) {
                            resultList.add(fractionalRmsPowerDeviation * 100);
                        }
                        
                    }
                    double rmsJitter = 0.0;
                    if (jitterList.size() > 2 * TRIM) {
                        double iat = 0.0;
                        count = entries = 0;
                        for (Long jitter : jitterList) {
                            if (count > TRIM - 1 && count < jitterList.size() - TRIM) {
                                iat += jitter.longValue();
                                entries++;
                            }
                            count++;
                        }
                        double averageIat = (double)iat / (double)entries;
                        double jitterSquared = 0.0;
                        entries = count = 0;
                        for (Long jitter: jitterList) {
                            if (count > TRIM - 1 && count < jitterList.size() - TRIM) {
                                double diff = (double)(jitter.longValue()) - averageIat;
                                jitterSquared += (diff*diff);
                                entries++;
                            }
                            count++;
                        }
                       
                        if ( entries > 0 ) {
                            rmsJitter = (double)jitterSquared/(double)entries;
                        }
                        jitterResultList.add(rmsJitter);
                        
                        
                    }
                    System.out.println("count = " + count + " power standard deviation "
                            + fractionalRmsPowerDeviation + " jitter " + rmsJitter);

                }

                sendBye();
            } catch (Exception ex) {
                logger.error("Unexpected exception", ex);
            }
        }

    }

    class Receiver implements Runnable {

        private String ipAddress;
        private int port;

        public Receiver(String ipAddress, int port) {
            System.out.println("port = " + port);
            this.ipAddress = ipAddress;
            this.port = port;
        }

        public void run() {
            try {

                SocketAddress socketAddress = new InetSocketAddress(this.ipAddress, this.port);
                byte[] buffer = new byte[1024];
                DatagramPacket datagramPacket = new DatagramPacket(buffer, buffer.length);
                
                int counter = 0;
                long sum = 0;
                long nbytes = 0;
                double power = 0;
                long previousTime = System.currentTimeMillis();
                while (true) {
                    counter++;
                    datagramSocket.receive(datagramPacket);
                    long time = System.currentTimeMillis();
                    if (isComputeClient()) {

                        byte[] data = datagramPacket.getData();
                        int length = datagramPacket.getLength();
                        // Skip the header and get the payload.
                       
                        for (int i = 12; i < length; i++) {
                            int val ;
                            if ( (int) data[i] < 0  ) {
                                val = ((int)0x7F & data[i]) + ((int)0x80);
                            } else {
                                val = (int) data[i];
                            }
                            int diff = 255 - val;
                            
                            nbytes += length;
                            sum = sum + diff * diff;
                        }
                       
                        if (counter % 10 == 0) {

                            power =  ((double) sum / (double) nbytes);
                            powerList.add(power);
                            nbytes = 0;
                            sum = 0;
                        }
                        long diff = time - previousTime;
                        previousTime = time;
                        jitterList.add(diff);
                    }

                }

            } catch (Exception ex) {
                ex.printStackTrace();
                logger.error("Unepxected exception ", ex);
            }

        }

    }

    public void sendInvite() {
        System.out.println("Sending INVITE " + FreeSwitchTester.getClientId() + ":"
                + this.testerId);
        try {
            Request inviteRequest = SipUtilities.createInviteRequest(provider, userName);

            this.mediaPort = FreeSwitchTester.getLastAllocatedMediaPort();
            ClientTransaction ctx = provider.getNewClientTransaction(inviteRequest);
            this.dialog = ctx.getDialog();
            ctx.sendRequest();
        } catch (Exception ex) {
            ex.printStackTrace();
            logger.error("Exception sending INVITE ", ex);
            throw new FreeSwitchTesterException("Exception sending INVITE ", ex);
        }

    }

    public void sendBye() throws Exception {
        Request byeRequest = dialog.createRequest(Request.BYE);
        ClientTransaction ctx = this.provider.getNewClientTransaction(byeRequest);
        dialog.sendRequest(ctx);
    }

    public ConferenceTestClient(int userId, SipProvider provider, int port) {
        String userName = FreeSwitchTester.getClientId() + "-" + userId;
        this.testerId = userId;
        this.provider = provider;
        this.port = port;
        this.userName = userName;
        this.listeningPoint = provider.getListeningPoint("udp");
        this.ipAddress = this.listeningPoint.getIPAddress();
    }

    public void processDialogTerminated(DialogTerminatedEvent arg0) {

    }

    public void processIOException(IOExceptionEvent ioExceptionEvent) {
        logger.error("IOException event " + ioExceptionEvent.getHost() + ":"
                + ioExceptionEvent.getPort());

    }

    public void processRequest(RequestEvent requestEvent) {
        logger.error("Got an unexpected RequestEvent " + requestEvent.getRequest());
    }

    public void processResponse(ResponseEvent responseEvent) {
        try {
            Response response = responseEvent.getResponse();
            if (responseEvent.getClientTransaction() == null) {
                return;
            }
            // System.out.println("got response = "+ response.getStatusCode());
            if (response.getContentLength().getContentLength() > 0 && !sdpReceived) {
                this.sdpReceived = true;
                SessionDescription sd = SipUtilities.getSessionDescription(response);
                this.remoteIpAddress = SipUtilities.getSessionDescriptionMediaIpAddress(sd);
                this.remoteMediaPort = SipUtilities.getSessionDescriptionMediaPort(sd);

                datagramSocket = new DatagramSocket(this.mediaPort,InetAddress.getByName(this.ipAddress));

                new Thread(new Receiver(this.ipAddress, mediaPort)).start();

                // Thread.sleep(100);

                if (FreeSwitchTester.getTesterConfig().getTesterClientConfig().getMediaFile() == null) {

                    String[] mediaFiles = new String[] {
                        "testmedia/500hz.au", "testmedia/750hz.au", "testmedia/1khz.au",
                        "testmedia/whitenoise.au"
                    };
                    new Thread(new MediaSender(isSender ? mediaFiles[this.testerId
                            % mediaFiles.length] : "testmedia/silence.au")).start();

                } else {
                    String mediaFile = FreeSwitchTester.getTesterConfig().getTesterClientConfig()
                            .getMediaFile();
                    new Thread(new MediaSender(mediaFile)).start();
                }

            }
            if (response.getStatusCode() == Response.OK && 
                    ((CSeqHeader) response.getHeader(CSeqHeader.NAME)).getMethod().equals(Request.INVITE)) {
                Dialog dialog = responseEvent.getDialog();
                long cseqNumber = ((CSeqHeader) response.getHeader(CSeqHeader.NAME))
                        .getSeqNumber();
                Request ack = dialog.createAck(cseqNumber);
                dialog.sendAck(ack);
                if (okCount.incrementAndGet() == FreeSwitchTester.getTesterConfig()
                        .getConferenceSize()) {
                    Thread.sleep(100);
                    synchronized (okCount) {
                        okCount.notifyAll();
                    }
                }
                System.out.println("OK count = " + okCount + " clientId " + this.testerId);

            }

        } catch (Exception ex) {
            throw new RuntimeException("Error processing response", ex);
        }
    }

    public void processTimeout(TimeoutEvent arg0) {

    }

    public void processTransactionTerminated(TransactionTerminatedEvent arg0) {

    }

    public boolean isComputeClient() {
        return !this.isSender;
    }

    public void setSender(boolean b) {
        System.out.println("ClientId = " + this.testerId + " isSender = " + b);
        this.isSender = b;
    }

}
