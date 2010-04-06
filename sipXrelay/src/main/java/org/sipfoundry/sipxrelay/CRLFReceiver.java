/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
/**
 * CRLF receiver for symmitron server. Responds to ping from 
 * proxy server.
 */
package org.sipfoundry.sipxrelay;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.concurrent.Semaphore;

import org.apache.log4j.Logger;

class CRLFReceiver implements Runnable {

    private static Logger logger = Logger.getLogger(CRLFReceiver.class);

    DatagramSocket pingSocket;
    boolean packetRecieved;    
    private long startTime;
    private byte[] buffer;
    private byte[] receiveBuffer;

    public CRLFReceiver() throws Exception {
        try {
            this.packetRecieved = false;
            pingSocket = new DatagramSocket();
            this.buffer = new String("\r\n\r\n").getBytes();  
            this.receiveBuffer = new String("\r\n\r\n").getBytes();
         } catch (Exception ex) {
            logger.error("Exception caught when creating CRLF Receiver", ex);
        }
    }

    public void sendPing(String host, int port) {
        try {
            packetRecieved = false;
            InetAddress proxyAddr = InetAddress.getByName(host);
            DatagramPacket sendDatagramPacket = new DatagramPacket(buffer,
                    buffer.length, proxyAddr, port);
            startTime = System.currentTimeMillis();
            pingSocket.send(sendDatagramPacket);
        } catch (Exception ex) {
            logger.error("Unexpected exception", ex);
        }
    }

    public void run() {
        while (true) {
             try {
                DatagramPacket receiveDatagramPacket = new DatagramPacket(receiveBuffer,receiveBuffer.length);        
                pingSocket.receive(receiveDatagramPacket);
            } catch (Exception ex) {
                logger.fatal("Could not get response from ProxyServer ", ex);
                logger.fatal("Exit Pinger Thread!");
                pingSocket.close();
                SymmitronServer.crlfReceiver = null;
                return;
            }
            long endTime = System.currentTimeMillis();
            if ( logger.isDebugEnabled() ) {
                logger.debug("received message Wait time =  "
                    + (endTime - startTime));
            }
            packetRecieved = true;
        }

    }

}