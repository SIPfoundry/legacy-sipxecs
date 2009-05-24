/**
 * CRLF receiver for symmitron server. Responds to ping from 
 * proxy server.
 */
package org.sipfoundry.sipxbridge.symmitron;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.concurrent.Semaphore;

class CRLFReceiver implements Runnable {
    Semaphore waitSem;
    DatagramSocket pingSocket;
    boolean packetRecieved;

    public CRLFReceiver() throws Exception {
        try {
            this.waitSem = new Semaphore(0);
            this.packetRecieved = false;
            pingSocket = new DatagramSocket();
        } catch (Exception ex) {
            SymmitronServer.logger
                    .error("Exception caught when creating CRLF Receiver",
                            ex);
        }
    }

    public void run() {
        while (true) {
            byte[] buffer = new String("\r\n\r\n").getBytes();
            DatagramPacket datagramPacket = new DatagramPacket(buffer,
                    buffer.length);
            waitSem.release();
            long startTime = System.currentTimeMillis();
            try {
                pingSocket.receive(datagramPacket);
            } catch (Exception ex) {
                SymmitronServer.logger.error("Could not get response from ProxyServer "
                        + ex.getMessage());
                return;
            }
            long endTime = System.currentTimeMillis();
            SymmitronServer.logger.debug("received message Wait time =  "
                    + (endTime - startTime));
            packetRecieved = true;
        }

    }

}