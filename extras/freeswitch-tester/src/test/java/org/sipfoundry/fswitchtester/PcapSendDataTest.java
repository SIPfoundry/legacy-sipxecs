package org.sipfoundry.fswitchtester;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketAddress;


import junit.framework.TestCase;

public class PcapSendDataTest extends TestCase {

    class Listener implements Runnable {

        DatagramSocket socket;
        int port;

        public Listener(String ipAddress, int port) throws Exception {
            SocketAddress socketAddress = new InetSocketAddress(ipAddress, port);
            socket = new DatagramSocket(socketAddress);
            this.port = port;
        }

        public void run() {
            byte[] buffer = new byte[1024];
            DatagramPacket packet = new DatagramPacket(buffer, 1024);
            while (true) {
                try {
                    socket.receive(packet);
                    System.out.println("Got something" + port);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }

        }

    }

    public void testSendData() throws Exception {
        final String ipAddress = "192.168.5.100";
        int targetPort = 49150;
        int srcPort = 48150;
       // Thread thread = new Thread(new Listener(ipAddress, targetPort));
        //thread.start();
        PcapRtpStream rtpStream = new PcapRtpStream("testmedia/capture.pcap", "192.168.5.242",
                2228, "192.168.5.240", 28266, "192.168.5.240", srcPort, ipAddress, targetPort);
        rtpStream.send();
        Thread.sleep(30000);
        rtpStream.close();
        //thread.interrupt();
    }

}
