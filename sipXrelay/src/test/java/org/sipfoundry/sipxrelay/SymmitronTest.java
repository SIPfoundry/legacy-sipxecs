/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.Map;

import org.sipfoundry.sipxrelay.BridgeState;
import org.sipfoundry.sipxrelay.SymInterface;
import org.sipfoundry.sipxrelay.Symmitron;

import junit.framework.TestCase;

public class SymmitronTest extends AbstractSymmitronTestCase {

    int counter = 0;

    Thread listener1;
    Thread listener2;

    class Listener implements Runnable {

        private DatagramSocket datagramSocket;

        public Listener(DatagramSocket datagramSocket) {
            this.datagramSocket = datagramSocket;
        }

        public void run() {
            while (true) {
                try {
                    byte[] data = new byte[1024];
                    DatagramPacket datagramPacket = new DatagramPacket(data, data.length);
                    this.datagramSocket.receive(datagramPacket);
                    counter++;
                    // System.out.println("Got something ");
                } catch (IOException ex) {
                    return;
                } catch (Exception ex) {
                    TestCase.fail("Unexpected IO exception ");
                }
            }

        }

    }

    public void setUp() throws Exception {
        super.setUp();
        super.start();
        super.signIn();

    }

    public void testSignInSignOut() {
        try {
            super.start();
            String[] myHandle = new String[1];
            myHandle[0] = clientHandle;
            Map retval = (Map) client.execute("sipXrelay.signIn", (Object[]) myHandle);
            client.execute("sipXrelay.signOut", (Object[]) myHandle);

        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception ");
        }
    }
    public void testSelfRouting() throws Exception {
        int destinationPort1 = 26000;
        int destinationPort2 = 27000;

        String sym1 = super.createEvenSym();
        String sym2 = super.createEvenSym();

        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym1;
        Map retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = (Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port1 = (Integer) receiverSession.get("port");

        args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym2;
        retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        receiverSession = (Map) symSession.get("receiver");
        ipAddr = (String) receiverSession.get("ipAddress");
        int port2 = (Integer) receiverSession.get("port");

        String bridge1 = super.createBridge();
        
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge1;
        args[2] = sym1;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge1;
        args[2] = sym2;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);
       

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge1;
        args[2] = sym1;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge1;
        args[2] = sym2;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        // Create a second bridge with two syms.
        String sym3 = super.createEvenSym();
      

        String sym4 = super.createEvenSym();
         
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym3;
        retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        receiverSession = (Map) symSession.get("receiver");
        ipAddr = (String) receiverSession.get("ipAddress");
        int port3 = (Integer) receiverSession.get("port");
       
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym4;
        retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        receiverSession = (Map) symSession.get("receiver");
        ipAddr = (String) receiverSession.get("ipAddress");
        int port4 = (Integer) receiverSession.get("port");
        String bridge2 = super.createBridge();
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge2;
        args[2] = sym3;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge2;
        args[2] = sym4;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);
        
        
        
        
        

        super.setRemoteEndpoint(sym3, port2);
        super.setRemoteEndpoint(sym2, port3);
        super.setRemoteEndpoint(sym1, destinationPort1);
        super.setRemoteEndpoint(sym4,destinationPort2);
         
        
        
        

        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();

       
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge1;
        retval = (Map) client.execute("sipXrelay.startBridge", args);

        super.checkStandardMap(retval);
        
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge2;
        retval = (Map) client.execute("sipXrelay.startBridge", args);

        super.checkStandardMap(retval);
        
        Thread.sleep(2000);

        assertTrue("Should see a non zero counter", counter != 0);


        this.counter = 0;

        byte[] data = new byte[1024];
        DatagramSocket datagramSocket = datagramSocket1;
        System.out.println("Sending to " + port1);
        DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                .getByName(ipAddr), port1);
        for (int i = 0; i < 1000; i++) {

            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue("Counter is " + counter, counter >= 1000);
        this.counter = 0;
        datagramPacket = new DatagramPacket(data, data.length, InetAddress.getByName(ipAddr),
                port4);
        System.out.println("Sending to " + port4);
        datagramSocket = datagramSocket2;
        for (int i = 0; i < 1000; i++) {
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        
        Thread.sleep(100);
        
        assertTrue("Counter is " + counter, counter >= 1000);

        datagramSocket1.close();
        datagramSocket2.close();
      

        super.destroyBridge(bridge1);
        super.destroyBridge(bridge2);
    }
    public void testSymCreate() throws Exception {

        int count = 1;
        Object[] args = new Object[3];
        args[0] = clientHandle;
        args[1] = new Integer(count);
        args[2] = Symmitron.EVEN;

        Map retval = (Map) super.client.execute("sipXrelay.createSyms", args);
        super.checkStandardMap(retval);
        Object[] syms = (Object[]) retval.get(Symmitron.SYM_SESSION);
        assertEquals("Should allocate only one sym", syms.length, count);
        System.out.println("syms = " + syms);
        System.out.println("sym[0] = " + (Map) syms[0]);
        String symId = (String) ((Map) syms[0]).get("id");

        args = new Object[2];
        args[0] = clientHandle;
        args[1] = symId;
        retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = (Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port = (Integer) receiverSession.get("port");
        System.out.println("ipAddr = " + ipAddr);
        System.out.println("port = " + port);

        args = new Object[2];
        args[0] = clientHandle;
        args[1] = symId;
        retval = (Map) super.client.execute("sipXrelay.destroySym", args);
        System.out.println("retval = " + retval);
        super.checkStandardMap(retval);

    }

    public void testCreateBridge() throws Exception {
        Object[] args = new Object[1];
        args[0] = clientHandle;
        Map retval = (Map) super.client.execute("sipXrelay.createBridge", args);
        super.checkStandardMap(retval);
        assertNotNull(retval.get(Symmitron.BRIDGE_ID));
        String bridgeId = (String) retval.get(Symmitron.BRIDGE_ID);
        args = new Object[2];
        args[0] = this.clientHandle;
        args[1] = bridgeId;
        retval = (Map) super.client.execute("sipXrelay.destroyBridge", args);
        super.checkStandardMap(retval);

    }

    public void testHeartBeat() throws Exception {
        int destinationPort1 = 26000;
        int destinationPort2 = 27000;

        String sym1 = super.createEvenSym();
        String sym2 = super.createEvenSym();
        String bridge = super.createBridge();

        Object[] params = new Object[3];
        params[0] = clientHandle;
        params[1] = bridge;
        params[2] = sym1;
        Map retval = (Map) client.execute("sipXrelay.addSym", params);
        super.checkStandardMap(retval);

        params = new Object[3];
        params[0] = clientHandle;
        params[1] = bridge;
        params[2] = sym2;
        retval = (Map) client.execute("sipXrelay.addSym", params);
        super.checkStandardMap(retval);
        
      

        /*
         * setDestination(String controllerHandle, String symId, String ipAddress, int port, int
         * keepAliveTime, String keepaliveMethod, byte[] keepAlivePacketData, boolean
         * autoDiscoverFlag)
         */

        params = new Object[6];
        params[0] = clientHandle;
        params[1] = sym1;
        params[2] = serverAddress;
        params[3] = new Integer(destinationPort1);
        params[4] = new Integer(500);
        params[5] = "USE-EMPTY-PACKET";

        retval = (Map) client.execute("sipXrelay.setDestination", params);
        super.checkStandardMap(retval);

        params = new Object[6];
        params[0] = clientHandle;
        params[1] = sym2;
        params[2] = serverAddress;
        params[3] = new Integer(destinationPort2);
        params[4] = new Integer(500);
        params[5] = "USE-EMPTY-PACKET";

        retval = (Map) client.execute("sipXrelay.setDestination", params);
        super.checkStandardMap(retval);

        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();
        
        super.startBridge(bridge);

        Thread.sleep(10000);

      
        datagramSocket1.close();
        datagramSocket2.close();

        super.destroyBridge(bridge);
        
        assertTrue("Should see a non zero counter", counter != 0);

    }

    /**
     * Create and send data through bridge.
     * 
     * @throws Exception
     */
    public void testSendDataThroughBridge() throws Exception {
        int destinationPort1 = 26000;
        int destinationPort2 = 27000;

        String sym1 = super.createEvenSym();
        String sym2 = super.createEvenSym();

        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym1;
        Map retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = (Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port1 = (Integer) receiverSession.get("port");

        args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym2;
        retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        receiverSession = (Map) symSession.get("receiver");
        ipAddr = (String) receiverSession.get("ipAddress");
        int port2 = (Integer) receiverSession.get("port");

        String bridge = super.createBridge();
        super.setRemoteEndpoint(sym1, destinationPort1);
        super.setRemoteEndpoint(sym2, destinationPort2);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym1;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();

       
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXrelay.startBridge", args);

        super.checkStandardMap(retval);
        Thread.sleep(2000);

        assertTrue("Should see a non zero counter", counter != 0);

        this.counter = 0;

        byte[] data = new byte[1024];
        DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                .getByName(ipAddr), port1);
        for (int i = 0; i < 1000; i++) {

            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue("Counter is " + counter, counter >= 1000);
        this.counter = 0;
        datagramPacket = new DatagramPacket(data, data.length,InetAddress
                .getByName(ipAddr), port2);
        for (int i = 0; i < 1000; i++) {
            Thread.sleep(10);
            datagramSocket2.send(datagramPacket);
        }
        
        Thread.sleep(100);
        assertTrue("Counter is " + counter, counter >= 1000);

        datagramSocket1.close();
        datagramSocket2.close();
      
        super.destroyBridge(bridge);

    }

    
    /**
     * Create and send data through bridge.
     * 
     * @throws Exception
     */
    public void testSendDataThroughBridgeDiscardStrayPackets() throws Exception {
        int destinationPort1 = 26000;
        int destinationPort2 = 27000;

        String sym1 = super.createEvenSym();
        String sym2 = super.createEvenSym();

        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym1;
        Map retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = (Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port1 = (Integer) receiverSession.get("port");

        args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym2;
        retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        receiverSession = (Map) symSession.get("receiver");
        ipAddr = (String) receiverSession.get("ipAddress");
        int port2 = (Integer) receiverSession.get("port");

        String bridge = super.createBridge();
        super.setRemoteEndpointNoKeepalive(sym1, destinationPort1);
        super.setRemoteEndpointNoKeepalive(sym2, destinationPort2);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym1;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();

       
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXrelay.startBridge", args);

        super.checkStandardMap(retval);
        Thread.sleep(2000);

      
        this.counter = 0;

        byte[] data = new byte[1024];
        DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                .getByName(ipAddr), port1);
        for (int i = 0; i < 100; i++) {

            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        Thread.sleep(100);
        
           
        this.counter = 0;
        datagramPacket = new DatagramPacket(data, data.length, InetAddress.getByName(ipAddr),
                port2);
        
        
        for (int i = 0; i < 100; i++) {
            Thread.sleep(10);
            datagramSocket2.send(datagramPacket);
        }
        Thread.sleep(100);
        int counter = this.counter;
        
        
        
        DatagramSocket strayPacketSocket = new DatagramSocket(new InetSocketAddress(InetAddress.getByName(super.testerAddress),7784));
     
        for (int i = 0; i < 1000; i++) {

            Thread.sleep(10);
            strayPacketSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertEquals("Counter should not change",counter, this.counter );
        strayPacketSocket.close();
        datagramSocket1.close();
        datagramSocket2.close();
      
        super.destroyBridge(bridge);

    }
    public void testSendDataThroughBridgeRemoteAddressAutoDiscovered() throws Exception {
        int destinationPort1 = 26000;
        int destinationPort2 = 27000;

        String sym1 = super.createEvenSym();
        String sym2 = super.createEvenSym();

        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym2;
        Map retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);

        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = (Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port = (Integer) receiverSession.get("port");

        String bridge = super.createBridge();
        super.setRemoteEndpoint(sym1, destinationPort1);
        super.setAutoDiscover(sym2);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym1;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();

      
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXrelay.startBridge", args);

        super.checkStandardMap(retval);
        Thread.sleep(2000);

        assertTrue("Should see a non zero counter", counter != 0);

        this.counter = 0;

        byte[] data = new byte[1024];
        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket2.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue(" Counter is " + counter, counter >= 1000);
        counter = 0;
        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue(counter >= 1000);

        datagramSocket1.close();
        datagramSocket2.close();

        super.destroyBridge(bridge);

    }

    /**
     * Test pausing and resuming bridge.
     * 
     * @throws Exception
     */
    public void testPauseAndResumeBridge() throws Exception {
        int destinationPort1 = 26000;
        int destinationPort2 = 27000;

        String sym1 = super.createEvenSym();
        String sym2 = super.createEvenSym();

        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym1;
        Map retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = (Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port = (Integer) receiverSession.get("port");

        System.out.println("ipAddr = " + ipAddr + " port " + port);

        String bridge = super.createBridge();
        super.setRemoteEndpoint(sym1, destinationPort1);
        super.setRemoteEndpoint(sym2, destinationPort2);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym1;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();

       
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXrelay.startBridge", args);

        super.checkStandardMap(retval);
        Thread.sleep(2000);

        assertTrue("Should see a non zero counter", counter != 0);

        this.counter = 0;

        byte[] data = new byte[1024];
        DatagramSocket datagramSocket = datagramSocket1;
        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue("couner must exceed 1000",counter >= 1000);

        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXrelay.pauseBridge", args);
        super.checkStandardMap(retval);

        this.counter = 0;

        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue(counter < 1000);

        super.resumeBridge(bridge);
        counter = 0;
        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue(counter >= 1000);

        datagramSocket1.close();
        datagramSocket2.close();

        super.destroyBridge(bridge);

    }

    public void testRemoveSymFromBridge() throws Exception {
        int destinationPort1 = 26000;
        int destinationPort2 = 27000;

        String sym1 = super.createEvenSym();
        String sym2 = super.createEvenSym();

        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym1;
        Map retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = (Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port = (Integer) receiverSession.get("port");

        System.out.println("ipAddr = " + ipAddr + " port " + port);

        String bridge = super.createBridge();
        super.setRemoteEndpoint(sym1, destinationPort1);
        super.setRemoteEndpoint(sym2, destinationPort2);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym1;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();

        super.startBridge(bridge);
        super.checkStandardMap(retval);

        this.counter = 0;

        byte[] data = new byte[1024];
        DatagramSocket datagramSocket = datagramSocket1;
        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }

        Thread.sleep(100);

        assertTrue(counter >= 1000);
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXrelay.removeSym", args);

        this.counter = 0;

        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue(counter < 100);
        super.destroySym(sym2);
        super.destroyBridge(bridge);
        datagramSocket1.close();
        datagramSocket2.close();

        Thread.sleep(100);

    }

    public void testPauseResumeSym() throws Exception {

        int destinationPort1 = 26000;
        int destinationPort2 = 27000;
        String sym1 = super.createEvenSym();
        String sym2 = super.createEvenSym();
        SymInterface sym = super.getSym(sym1);
        String ipAddr = sym.getReceiver().getIpAddress();
        int port = sym.getReceiver().getPort();

        String bridge = super.createBridge();
        super.setRemoteEndpoint(sym1, destinationPort1);
        super.setRemoteEndpoint(sym2, destinationPort2);
        super.addSym(bridge, sym1);
        super.addSym(bridge, sym2);

        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();

        super.startBridge(bridge);

        this.counter = 0;

        byte[] data = new byte[1024];
        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue(counter >= 1000);

        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym2;
        Map retval = (Map) client.execute("sipXrelay.pauseSym", args);
        super.checkStandardMap(retval);
        this.counter = 0;

        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        Thread.sleep(100);

        // Heartbeat keeps going but no data.
        assertTrue(counter < 100);

        args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym2;
        retval = (Map) client.execute("sipXrelay.resumeSym", args);
        super.checkStandardMap(retval);
        this.counter = 0;

        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        Thread.sleep(100);

        // Heartbeat keeps going but no data.
        assertTrue(counter >= 1000);
        super.destroyBridge(bridge);
        datagramSocket1.close();
        datagramSocket2.close();

    }

    public void testPing() throws Exception {
        Object[] args = new Object[1];
        args[0] = clientHandle;

        String sym1 = super.createEvenSym();
        String sym2 = super.createEvenSym();
        String bridge = super.createBridge();

        Map retval = (Map) client.execute("sipXrelay.ping", args);

        Object[] timedOutSyms = (Object[]) retval.get(Symmitron.SYM_SESSION);

        assertNull(timedOutSyms);

        super.addSym(bridge, sym1);
        super.addSym(bridge, sym2);
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = sym2;
        args[2] = 1000;
        retval = (Map) client.execute("sipXrelay.setTimeout", args);
        Thread.sleep(2000);
        args = new Object[1];
        args[0] = clientHandle;

        retval = (Map) client.execute("sipXrelay.ping", args);

        timedOutSyms = (Object[]) retval.get(Symmitron.SYM_SESSION);

        assertTrue(timedOutSyms.length == 1);
        assertTrue(timedOutSyms[0].equals(sym2));

        int destinationPort1 = 26000;
        SymInterface sym = super.getSym(sym1);
        String ipAddr = sym.getReceiver().getIpAddress();
        int port = sym.getReceiver().getPort();
        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                .getByName(serverAddress));
        byte[] data = new byte[1024];

        for (int i = 0; i < 1000; i++) {
            DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                    .getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        args = new Object[1];
        args[0] = clientHandle;

        retval = (Map) client.execute("sipXrelay.ping", args);
        assertTrue(timedOutSyms.length == 1);
        assertTrue(timedOutSyms[0].equals(sym2));

        super.destroyBridge(bridge);
        datagramSocket1.close();

    }

    /**
     * Create and send data through bridge.
     * 
     * @throws Exception
     */
    private String sendDataThroughBridgeNoDestroy() throws Exception {
        int destinationPort1 = 26000;
        int destinationPort2 = 27000;

        String sym1 = super.createEvenSym();
        String sym2 = super.createEvenSym();

        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym1;
        Map retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = (Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port1 = (Integer) receiverSession.get("port");

        args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym2;
        retval = (Map) client.execute("sipXrelay.getSym", args);
        super.checkStandardMap(retval);
        symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        receiverSession = (Map) symSession.get("receiver");
        ipAddr = (String) receiverSession.get("ipAddress");
        int port2 = (Integer) receiverSession.get("port");

        String bridge = super.createBridge();
        
        
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXrelay.getBridgeStatistics", args);
        
        String bridgeState  = (String) retval.get(Symmitron.BRIDGE_STATE);
        assertTrue("Must be in INITIAL state",bridgeState.equals(BridgeState.INITIAL.toString()));        
        String packetsReceived  = ( String ) retval.get(Symmitron.PACKETS_RECEIVED);
        assertNotNull(packetsReceived);
        assertEquals("Must be 0 packets received",Integer.parseInt(packetsReceived),0);
        String packetsSent  = ( String ) retval.get(Symmitron.PACKETS_SENT);
        assertNotNull(packetsSent);
        assertEquals("Must be 0 packets received",Integer.parseInt(packetsSent),0);
        
        
        super.setRemoteEndpoint(sym1, destinationPort1);
        super.setRemoteEndpoint(sym2, destinationPort2);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym1;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXrelay.addSym", args);
        super.checkStandardMap(retval);

        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2, InetAddress
                .getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();

       
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXrelay.startBridge", args);
        Thread.sleep(2000);

        assertTrue("Should see a non zero counter", counter != 0);

        super.checkStandardMap(retval);

        this.counter = 0;

        byte[] data = new byte[1024];
        DatagramSocket datagramSocket = datagramSocket1;
        DatagramPacket datagramPacket = new DatagramPacket(data, data.length, InetAddress
                .getByName(ipAddr), port1);
        for (int i = 0; i < 1000; i++) {

            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue("Counter is " + counter, counter >= 1000);
        this.counter = 0;
        datagramPacket = new DatagramPacket(data, data.length, InetAddress.getByName(ipAddr),
                port2);
        for (int i = 0; i < 1000; i++) {
            Thread.sleep(10);
            datagramSocket2.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue("Counter is " + counter, counter >= 1000);
        
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXrelay.getBridgeStatistics", args);
        
        retval = (Map) client.execute("sipXrelay.getBridgeStatistics", args);
        
        bridgeState  = (String) retval.get(Symmitron.BRIDGE_STATE);
        assertTrue("Must be in INITIAL state",bridgeState.equals(BridgeState.RUNNING.toString()));        
        packetsReceived  = ( String ) retval.get(Symmitron.PACKETS_RECEIVED);
        assertNotNull(packetsReceived);
        assertEquals("Must be 2000 packets received",Integer.parseInt(packetsReceived),2000);
        packetsSent  = ( String ) retval.get(Symmitron.PACKETS_SENT);
        assertNotNull(packetsSent);
        assertEquals("Must be 2000 packets received",Integer.parseInt(packetsSent),2000);
        
        System.out.println("Pakets sent = " + packetsSent + " packets received " + packetsReceived);
      
        datagramSocket1.close();
        datagramSocket2.close();
        
        return bridge;

    }

    public void testStartStop() throws Exception {
        this.sendDataThroughBridgeNoDestroy();
        clientHandle = "nat:4567";
        super.signIn();
        this.sendDataThroughBridgeNoDestroy();
        clientHandle = "nat:5678";
        super.signIn();
        this.testSendDataThroughBridge();
    }

    public void testForResourceLeak() throws Exception {
        for (int j = 0; j < 10; j++) {

            for (int i = 0; i < 50; i++) {
                String sym1 = super.createEvenSym();
                String sym2 = super.createEvenSym();
                String bridge = super.createBridge();
                super.addSym(bridge, sym1);
                super.addSym(bridge, sym2);

            }
            clientHandle = "nat:" + 12345 + j + 1;
            super.signIn();
        }

    }

   
    
    public void testBridgeStatistics() throws Exception {
    
       String bridge =  this.sendDataThroughBridgeNoDestroy();
        System.out.println("Done!");
       super.destroyBridge(bridge);
     

    }
    
   

}
