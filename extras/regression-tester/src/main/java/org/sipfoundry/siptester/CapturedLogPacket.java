/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.siptester;

import gov.nist.javax.sip.message.MessageExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.SIPMessage;
import gov.nist.javax.sip.message.SIPRequest;
import gov.nist.javax.sip.message.SIPResponse;

import java.io.File;
import java.net.InetAddress;
import java.util.Hashtable;

import javax.sip.message.Message;
import javax.sip.message.Request;

/**
 * 
 */
public class CapturedLogPacket implements Comparable<CapturedLogPacket> {

    private long timeStamp;
    private String destinationAddress;
    private int destinationPort;
    private String sourceAddress;
    private int sourcePort;
    private MessageExt sipMessage;
    private File traceFile;
    
    public static Hashtable<String,HostPort> hostMapper = new Hashtable<String,HostPort>();
    

    private long parseTime(String time) {
        String timeStr1 = time.split("T")[1];
        String timeStr = timeStr1.substring(0, timeStr1.length() - 1);
        String[] timeStrSplit = timeStr.split(":");
        long hours = Integer.parseInt(timeStrSplit[0]) * 60 * 60 * 1000;
        long minutes = Integer.parseInt(timeStrSplit[1]) * 60 * 1000;
        String[] secondsMicroseconds = timeStrSplit[2].split("\\.");
        long seconds = Integer.parseInt(secondsMicroseconds[0]) * 1000;
        long nanoseconds = Integer.parseInt(secondsMicroseconds[1]) / 1000;
        return hours + minutes + seconds + nanoseconds;

    }

    public CapturedLogPacket() {

    }

    public void setTimeStamp(String timeStamp) {
        this.timeStamp = parseTime(timeStamp);    
    }

    public CapturedLogPacket( String logPacket, long timeStamp, 
            String remoteAddress, int remotePort, boolean isSender) throws Exception {
        this.timeStamp = timeStamp;
        this.setDestinationAddress(remoteAddress);
        if (logPacket.startsWith("SIP")) {
            this.sipMessage = (MessageExt) SipTester.getStackBean().getMessageFactory()
                    .createResponse(logPacket);
        } else {
            this.sipMessage = (MessageExt) SipTester.getStackBean().getMessageFactory()
                    .createRequest(logPacket);
        }

    }

    public void setMessage(String rawLogPacket) {
          String logPacket = rawLogPacket.trim() + "\r\n\r\n";
          try {
            if (logPacket.startsWith("SIP")) {
                this.sipMessage = (MessageExt) SipTester.getStackBean().getMessageFactory()
                        .createResponse(logPacket);
                
            } else {
                this.sipMessage = (MessageExt) SipTester.getStackBean().getMessageFactory()
                        .createRequest(logPacket);
                if ( sipMessage instanceof RequestExt) {
                    String sourceHost = sipMessage.getTopmostViaHeader().getHost();
                    int sourcePort = sipMessage.getTopmostViaHeader().getPort();
                    if ( sourcePort == -1) {
                        sourcePort = 5060;
                    }
                    HostPort hostPort = new HostPort(sourceHost,sourcePort);
                    if ( this.sourcePort == 0) {
                        hostMapper.put(this.sourceAddress,hostPort);
                    }
                    this.setSourceAddress(sourceHost);
                    this.setSourcePort(sourcePort);
                }
            }
            
        } catch (Exception ex) {
            throw new SipTesterException(ex);
        }
    }
    
    public MessageExt getMessage() {
        return this.sipMessage;
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.siptester.CapturedPacket#compareTo(java.lang.Object)
     */
    @Override
    public int compareTo(CapturedLogPacket other) {
        if (this.timeStamp == other.timeStamp)
            return 0;
        else if (this.timeStamp < other.timeStamp)
            return -1;
        else
            return 1;
    }

    public MessageExt getSipPacket() {
        return this.sipMessage;
    }

    public long getTimeStamp() {
        return this.timeStamp;
    }

    
    public String getTopmostViaHost() {
        return ((SIPMessage) sipMessage).getTopmostVia().getHost();
    }

    public int getTopmostViaPort() {
        return ((SIPMessage) sipMessage).getTopmostVia().getPort();
    }

    public String getTransactionId() {
        return ((SIPMessage) sipMessage).getTransactionId();
    }

    /**
     * @return the traceFile
     */
    public File getTraceFile() {
        return traceFile;
    }

    /**
     * @param remoteAddress the remoteAddress to set
     */
    public void setDestinationAddress(String remoteAddress) {
        if ( remoteAddress.indexOf(":") != -1 ) {
            String[] addressPort = remoteAddress.split(":");
            this.destinationAddress = addressPort[0];
            this.destinationPort = Integer.parseInt(addressPort[1]);
        } else {
            this.destinationAddress = remoteAddress;
        }
    }

    
    /**
     * @param sourceAddress the sourceAddress to set
     */
    public void setSourceAddress(String sourceAddress) {
        if ( sourceAddress.indexOf(":") != -1 ) {
            String[] addressPort = sourceAddress.split(":");
            this.sourceAddress = addressPort[0];
            this.sourcePort = Integer.parseInt(addressPort[1]);
            
        } else {
            this.sourceAddress = sourceAddress;
        }
    }
    
    

    /**
     * @return the sourceAddress
     */
    public String getSourceAddress() {
        return sourceAddress;
    }

    /**
     * @param sourcePort the sourcePort to set
     */
    public void setSourcePort(int sourcePort) {
        this.sourcePort = sourcePort;
    }

    /**
     * @return the sourcePort
     */
    public int getSourcePort() {
        return sourcePort;
    }

    public String getDestinationAddess() {
        return destinationAddress;
    }
    
    public int getDestinationPort() {
        return this.destinationPort;
    }

    public void setDestinationPort(int port) {
       this.destinationPort = port;
    }


}
