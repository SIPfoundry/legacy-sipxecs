/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Map;

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
                    DatagramPacket datagramPacket = new DatagramPacket(data,
                            data.length);
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
            String[] myHandle = new String[1] ;
            myHandle[0] = "nat:12345";
            Map retval = (Map) client.execute("sipXbridge.signIn",
                    (Object[]) myHandle);
            client.execute("sipXbridge.signOut",(Object[]) myHandle);
           
            
        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception ");
        }
    }
 public void testSymCreate() throws Exception {
        
        int count = 1;
        Object[] args = new Object[3];
        args[0] = clientHandle;
        args[1] = new Integer(count);
        args[2] = Symmitron.EVEN;
        
        Map retval = (Map) super.client.execute("sipXbridge.createSyms", args);
        super.checkStandardMap(retval);
        Object[] syms = (Object[]) retval.get(Symmitron.SYM_SESSION);
        assertEquals ("Should allocate only one sym", syms.length , count);
        System.out.println("syms = " + syms);
        System.out.println("sym[0] = " + (Map) syms[0]);
        String symId = (String)((Map)syms[0]).get("id");
      
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = symId;
        retval = (Map) client.execute("sipXbridge.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = ( Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port = (Integer) receiverSession.get("port");
        System.out.println("ipAddr = " + ipAddr);
        System.out.println("port = " + port);
       
        
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = symId;
        retval = ( Map ) super.client.execute("sipXbridge.destroySym",args);
        System.out.println("retval = "  + retval);
        super.checkStandardMap( retval);
       
        
    }
    
    public void testCreateBridge() throws Exception {
        Object[] args = new Object[1];
        args[0] = clientHandle;
        Map retval = (Map) super.client.execute("sipXbridge.createBridge", args);
        super.checkStandardMap(retval);
        assertNotNull ( retval.get(Symmitron.BRIDGE_ID));
        String bridgeId = (String) retval.get(Symmitron.BRIDGE_ID);
        args = new Object[2];
        args[0] = this.clientHandle;
        args[1] = bridgeId;
        retval = (Map) super.client.execute("sipXbridge.destroyBridge",args);
        super.checkStandardMap( retval);
        
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
        Map retval = (Map) client.execute("sipXbridge.addSym", params);
        super.checkStandardMap(retval);

        params = new Object[3];
        params[0] = clientHandle;
        params[1] = bridge;
        params[2] = sym2;
        retval = (Map) client.execute("sipXbridge.addSym", params);
        super.checkStandardMap(retval);

        /*
         * setDestination(String controllerHandle, String symId, String
         * ipAddress, int port, int keepAliveTime, String keepaliveMethod,
         * byte[] keepAlivePacketData, boolean autoDiscoverFlag)
         */

        params = new Object[8];
        params[0] = clientHandle;
        params[1] = sym1;
        params[2] = serverAddress;
        params[3] = new Integer(destinationPort1);
        params[4] = new Integer(500);
        params[5] = "USE-EMPTY-PACKET";
        params[6] = "";
        params[7] = new Boolean(false);

        retval = (Map) client.execute("sipXbridge.setDestination", params);
        super.checkStandardMap(retval);

        params = new Object[8];
        params[0] = clientHandle;
        params[1] = sym2;
        params[2] = serverAddress;
        params[3] = new Integer(destinationPort2);
        params[4] = new Integer(500);
        params[5] = "USE-EMPTY-PACKET";
        params[6] = "";
        params[7] = new Boolean(false);
        retval = (Map) client.execute("sipXbridge.setDestination", params);
        super.checkStandardMap(retval);

        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1,
                InetAddress.getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2,
                InetAddress.getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();

        Thread.sleep(10000);

        assertTrue("Should see a non zero counter", counter != 0);

        datagramSocket1.close();
        datagramSocket2.close();

        super.destroyBridge(bridge);
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
        Map retval = (Map) client.execute("sipXbridge.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = ( Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port = (Integer) receiverSession.get("port");
        
        
        
        String bridge = super.createBridge();
        super.setRemoteEndpoint(sym1, destinationPort1);
        super.setRemoteEndpoint(sym2,destinationPort2);
        
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym1;
        retval = (Map) client.execute("sipXbridge.addSym", args);
        super.checkStandardMap(retval);
        
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXbridge.addSym", args);
        super.checkStandardMap(retval);
       
        
        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1,
                InetAddress.getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2,
                InetAddress.getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();
       
        
        Thread.sleep(2000);

        assertTrue("Should see a non zero counter", counter != 0);
        
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXbridge.startBridge", args);
        
        super.checkStandardMap(retval);
        
        this.counter = 0;
        
        byte[] data  = new byte[1024]  ;
        DatagramSocket datagramSocket = new DatagramSocket();
        for ( int i = 0 ; i < 1000 ; i++) {
            DatagramPacket datagramPacket = new DatagramPacket( data, 
                    data.length,InetAddress.getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue ( counter >= 1000 );

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
        Map retval = (Map) client.execute("sipXbridge.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = ( Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port = (Integer) receiverSession.get("port");
        
        System.out.println("ipAddr = " + ipAddr + " port " + port);
        
        
        String bridge = super.createBridge();
        super.setRemoteEndpoint(sym1, destinationPort1);
        super.setRemoteEndpoint(sym2,destinationPort2);
        
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym1;
        retval = (Map) client.execute("sipXbridge.addSym", args);
        super.checkStandardMap(retval);
        
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXbridge.addSym", args);
        super.checkStandardMap(retval);
       
        
        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1,
                InetAddress.getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2,
                InetAddress.getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();
       
        
        Thread.sleep(2000);

        assertTrue("Should see a non zero counter", counter != 0);
        
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXbridge.startBridge", args);
        
        super.checkStandardMap(retval);
        
        this.counter = 0;
        
        byte[] data  = new byte[1024]  ;
        DatagramSocket datagramSocket = new DatagramSocket();
        for ( int i = 0 ; i < 1000 ; i++) {
            DatagramPacket datagramPacket = new DatagramPacket( data, 
                    data.length,InetAddress.getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue ( counter >= 1000 );
        
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        retval = (Map) client.execute("sipXbridge.pauseBridge",args);
        super.checkStandardMap(retval);
        
        this.counter = 0;
       
        for ( int i = 0 ; i < 1000 ; i++) {
            DatagramPacket datagramPacket = new DatagramPacket( data, 
                    data.length,InetAddress.getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue ( counter < 1000 );
        
        super.resumeBridge(bridge);
        counter = 0;
        for ( int i = 0 ; i < 1000 ; i++) {
            DatagramPacket datagramPacket = new DatagramPacket( data, 
                    data.length,InetAddress.getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue ( counter >= 1000 );

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
        Map retval = (Map) client.execute("sipXbridge.getSym", args);
        super.checkStandardMap(retval);
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = ( Map) symSession.get("receiver");
        String ipAddr = (String) receiverSession.get("ipAddress");
        int port = (Integer) receiverSession.get("port");
        
        System.out.println("ipAddr = " + ipAddr + " port " + port);
        
        
        String bridge = super.createBridge();
        super.setRemoteEndpoint(sym1, destinationPort1);
        super.setRemoteEndpoint(sym2,destinationPort2);
        
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym1;
        retval = (Map) client.execute("sipXbridge.addSym", args);
        super.checkStandardMap(retval);
        
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXbridge.addSym", args);
        super.checkStandardMap(retval);
       
        
        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1,
                InetAddress.getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2,
                InetAddress.getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();
       
        super.startBridge(bridge);
        super.checkStandardMap(retval);
        
        this.counter = 0;
        
        byte[] data  = new byte[1024]  ;
        DatagramSocket datagramSocket = new DatagramSocket();
        for ( int i = 0 ; i < 1000 ; i++) {
            DatagramPacket datagramPacket = new DatagramPacket( data, 
                    data.length,InetAddress.getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        
        
        Thread.sleep(100);
        
        
        assertTrue ( counter >= 1000 );
        args  = new Object[3];
        args[0] = clientHandle;
        args[1] = bridge;
        args[2] = sym2;
        retval = (Map) client.execute("sipXbridge.removeSym", args);
        
        this.counter = 0;
        
        for ( int i = 0 ; i < 1000 ; i++) {
            DatagramPacket datagramPacket = new DatagramPacket( data, 
                    data.length,InetAddress.getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue (counter < 100);
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
        super.setRemoteEndpoint(sym2,destinationPort2);
        super.addSym(bridge, sym1);
        super.addSym(bridge, sym2);
       
        
        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1,
                InetAddress.getByName(serverAddress));

        new Thread(new Listener(datagramSocket1)).start();

        DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2,
                InetAddress.getByName(serverAddress));

        new Thread(new Listener(datagramSocket2)).start();
       
        
        super.startBridge(bridge);
        
       
       
        
        this.counter = 0;
        
        byte[] data  = new byte[1024]  ;
        for ( int i = 0 ; i < 1000 ; i++) {
            DatagramPacket datagramPacket = new DatagramPacket( data, 
                    data.length,InetAddress.getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        Thread.sleep(100);
        assertTrue ( counter >= 1000 );
        
        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym2;
        Map retval = (Map) client.execute("sipXbridge.pauseSym", args);
        super.checkStandardMap(retval);
        this.counter = 0;
        
        for ( int i = 0 ; i < 1000 ; i++) {
            DatagramPacket datagramPacket = new DatagramPacket( data, 
                    data.length,InetAddress.getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        Thread.sleep(100);
        
        // Heartbeat keeps going but no data.
        assertTrue(counter < 100);
        
        
        args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym2;
        retval = (Map) client.execute("sipXbridge.resumeSym", args);
        super.checkStandardMap(retval);
        this.counter = 0;
        
        for ( int i = 0 ; i < 1000 ; i++) {
            DatagramPacket datagramPacket = new DatagramPacket( data, 
                    data.length,InetAddress.getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        Thread.sleep(100);
        
        // Heartbeat keeps going but no data.
        assertTrue(counter >= 1000 );
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
        
        Map retval = (Map) client.execute("sipXbridge.ping", args);
        
        Object[] timedOutSyms = (Object[]) retval.get(Symmitron.SYM_SESSION);
       
        assertNull( timedOutSyms);
   
        super.addSym(bridge, sym1);
        super.addSym(bridge,sym2);
        args = new Object[3];
        args[0] = clientHandle;
        args[1] = sym2;
        args[2] = 1000;
        retval = (Map) client.execute("sipXbridge.setTimeout",args);
        Thread.sleep(2000);
        args = new Object[1];
        args[0] = clientHandle;
    
        retval = (Map) client.execute("sipXbridge.ping", args);
        
        timedOutSyms = (Object[]) retval.get(Symmitron.SYM_SESSION);
        
        assertTrue ( timedOutSyms.length == 1) ;
        assertTrue (timedOutSyms[0].equals(sym2));
        
        int destinationPort1 = 26000;
        SymInterface sym = super.getSym(sym1);
        String ipAddr = sym.getReceiver().getIpAddress();
        int port = sym.getReceiver().getPort();
        DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1,
                InetAddress.getByName(serverAddress));
        byte[] data  = new byte[1024]  ;
        
        for ( int i = 0 ; i < 1000 ; i++) {
            DatagramPacket datagramPacket = new DatagramPacket( data, 
                    data.length,InetAddress.getByName(ipAddr), port);
            Thread.sleep(10);
            datagramSocket1.send(datagramPacket);
        }
        args = new Object[1];
        args[0] = clientHandle;
    
        retval = (Map) client.execute("sipXbridge.ping", args);
        assertTrue ( timedOutSyms.length == 1) ;
        assertTrue (timedOutSyms[0].equals(sym2));
     
       
        super.destroyBridge(bridge);
        datagramSocket1.close();
       
   
    }
    
}
