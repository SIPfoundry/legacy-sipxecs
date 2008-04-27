package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Map;

import junit.framework.TestCase;

public class SendDataThroughBridgeTest extends AbstractSymmitronTestCase {

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

        datagramSocket1.close();
        datagramSocket2.close();

        super.destroyBridge(bridge);

    }
}
