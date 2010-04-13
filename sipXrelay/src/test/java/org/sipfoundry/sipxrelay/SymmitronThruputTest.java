/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrelay;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.HashSet;

import org.sipfoundry.sipxrelay.SymEndpointInterface;
import org.sipfoundry.sipxrelay.SymInterface;
import org.sipfoundry.sipxrelay.SymmitronConfig;
import org.sipfoundry.sipxrelay.SymmitronConfigParser;

import junit.framework.TestCase;
import junit.textui.TestRunner;

public class SymmitronThruputTest extends AbstractSymmitronTestCase {

    HashSet<Listener> listeners = new HashSet<Listener>();
    HashSet<Transmitter> transmitters = new HashSet<Transmitter>();
    private static int npacket = 1000;
    int sleepTime = 20;
    private static int nbridges = 25;
    private static String testerAddress;
    private static int startPort = 42000;
    

    class Transmitter implements Runnable {
        String ipAddr;
        int port;
        int count;
        DatagramSocket datagramSocket;

        public Transmitter(DatagramSocket socket, String ipAddr, int port, int count) {
            this.ipAddr = ipAddr;
            this.port = port;
            this.count = count;
            this.datagramSocket = socket;
            System.out.println("Transmitter : hisPort = " + port + 
                   " myPort = "  + socket.getLocalPort());
        }

        public void run() {
            try {
                byte[] data = new byte[32];
                DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                        .getByName(ipAddr), port);
                for (int i = 0; i < npacket; i++) {

                    Thread.sleep(sleepTime);
                    datagramSocket.send(datagramPacket);
                }
            } catch (Exception ex) {
                System.out.println("Done!");
            }

        }

    }

    class Listener implements Runnable {
        int counter;
        private DatagramSocket datagramSocket;
        private long time = -1;
        private long delay = 0;
        private long lastCounter ;

        public Listener(DatagramSocket datagramSocket) {
            this.datagramSocket = datagramSocket;
        }

        public void run() {
            byte[] data = new byte[32];
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length);
            while (true) {
                try {

                    this.datagramSocket.receive(datagramPacket);
                    if (time == -1) {
                        time = System.currentTimeMillis();
                    } else {
                        long newtime = System.currentTimeMillis();
                        int delta = (int) (newtime - time);
                        delay += (delta - sleepTime) * (delta - sleepTime);
                        time = newtime;

                    }
                    counter++;

                } catch (IOException ex) {
                    return;
                } catch (Exception ex) {
                    TestCase.fail("Unexpected IO exception ");
                }
            }

        }

    }

    public void setUp() throws Exception {
        super.setName("testThruput10");
        super.connectToServer();
        super.start();
        super.signIn();

    }
    
    @Override public void tearDown() throws Exception {
        System.out.println("Done with thruput test!");
    }

    public void testThruput10() throws Exception {
        int destinationPort1 = startPort;
        int destinationPort2 = startPort;

        for (int i = 0; i < nbridges; i++) {
            String bridge = super.createBridge();
            String sym = super.createEvenSym();
            SymInterface symInterface = super.getSym(sym);
            SymEndpointInterface symEndpoint = symInterface.getReceiver();
            int port1 = symEndpoint.getPort();
            System.out.println("port1 = " + port1);
            destinationPort1 = startPort + i;
          
            DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                    .getByName(testerAddress));
            
            Listener listener1 = new Listener(datagramSocket1);
            this.listeners.add(listener1);
            super.setRemoteEndpointNoKeepAlive(sym,testerAddress, destinationPort1);
            super.addSym(bridge, sym);
            Transmitter transmitter1 = new Transmitter(datagramSocket1, serverAddress, port1,
                    npacket);
            
            sym = super.createEvenSym();
            symInterface = super.getSym(sym);
            symEndpoint = symInterface.getReceiver();
            int port2 = symEndpoint.getPort();
            System.out.println("port2 = " + port2);
            destinationPort2 = (startPort + i + nbridges);           
            
            DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2, InetAddress
                    .getByName(testerAddress));
            Listener listener2 = new Listener(datagramSocket2);
            this.listeners.add(listener2);
            super.setRemoteEndpointNoKeepAlive(sym, this.testerAddress, destinationPort2);
            super.addSym(bridge, sym);
            super.startBridge(bridge);
            
            
            Transmitter transmitter2 = new Transmitter(datagramSocket2, serverAddress, port2,
                    npacket);
            
            
            transmitters.add(transmitter1);
            transmitters.add(transmitter2);

        }
        for (Listener listener : this.listeners) {
            new Thread(listener).start();
            Thread.sleep(100);
        }

       
        for (Transmitter transmitter : this.transmitters) {
            new Thread(transmitter).start();
            Thread.sleep(100);

        }

	Thread.sleep(1000);

	while(true) {
	    boolean limitReached = true;
	    System.out.println("Check listeners!");
	    for (Listener listener : this.listeners) {
	        System.out.println("Count = " + listener.counter);
	        if (listener.counter == 0 ) limitReached = false; 
	        else if ( listener.counter != listener.lastCounter ) {
	            limitReached = false;
	            listener.lastCounter = listener.counter;
	        }

	    }
	    if ( limitReached) {

	        double jitter = 0;
	        for (Listener listener : this.listeners) {

	            long avgDelay = listener.delay / listener.counter;
	            double rmsDelay = Math.sqrt((double) avgDelay);
	            jitter += rmsDelay;
	        }
	        System.out.println("Packet Inter Arrival Time = " + sleepTime + " RMS Jitter " + jitter / listeners.size()); 
	        break;
	    } else {

	        Thread.sleep(5000);
	    }
	}

        for (Listener listener : this.listeners) {
            listener.datagramSocket.close();
        }
     
    }

    public static void main(String[] args) throws Exception {

        String nattraversalRulesDir = System.getProperties().getProperty("conf.dir");
        SymmitronConfigParser parser = new SymmitronConfigParser();
        String url = "file:" + nattraversalRulesDir + "/nattraversalrules.xml";

        SymmitronConfig symConfig = parser.parse(url);

        port = symConfig.getXmlRpcPort();
        serverAddress = symConfig.getLocalAddress();
        testerAddress = System.getProperties().getProperty("tester.address");
        npacket = Integer.parseInt(System.getProperties().getProperty("tester.npackets"));
        nbridges = Integer.parseInt(System.getProperties().getProperty("tester.callLoad"));
        String portRange = System.getProperties().getProperty("tester.startport");
        
        startPort = Integer.parseInt(portRange);
        
        TestRunner testRunner = new TestRunner();
        testRunner.doRun(new SymmitronThruputTest());

    }

}
