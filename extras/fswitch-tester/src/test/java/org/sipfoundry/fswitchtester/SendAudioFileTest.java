/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketAddress;

import junit.framework.TestCase;


public class SendAudioFileTest extends TestCase {
    class Listener implements Runnable {
        
        DatagramSocket socket ;
        int port ;
       
        
        public Listener( String ipAddress , int  port) throws Exception {
            SocketAddress socketAddress = new InetSocketAddress(ipAddress,port);
            socket = new DatagramSocket(socketAddress);
            this.port = port;
        }
        
        public void run() {
           byte[] buffer  = new byte[1024];
           DatagramPacket packet = new DatagramPacket(buffer,1024);
           while ( true ) {
              try {
                  socket.receive(packet) ;
                  System.out.println("Got something" + port);
              } catch (Exception e) {
                  e.printStackTrace();
                  break;
              }
           }
            
        }
        
    }

    public void testSendData() throws Exception {
        final String ipAddress = "192.168.5.240";
        int targetPort = 49150;
        Listener listener = new Listener(ipAddress,targetPort);
        Thread thread = new Thread( listener);
        thread.start();
        String targetAddress = "192.168.5.100";
       
        
        AudioFileStreamer rtpStream = new AudioFileStreamer("testmedia/500hz.au", 
               0L, listener.socket, targetAddress, targetPort);
        rtpStream.send();
        Thread.sleep(10000);
        listener.socket.close();
        thread.interrupt();
    }
}
