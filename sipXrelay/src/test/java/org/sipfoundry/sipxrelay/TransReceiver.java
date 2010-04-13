/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrelay;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.URL;
import java.util.HashSet;
import java.util.Map;
import java.util.Properties;

import junit.framework.TestCase;

import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.sipxrelay.SymEndpointImpl;
import org.sipfoundry.sipxrelay.SymEndpointInterface;
import org.sipfoundry.sipxrelay.SymImpl;
import org.sipfoundry.sipxrelay.SymInterface;
import org.sipfoundry.sipxrelay.Symmitron;

/*
 * The transmitter receiver pair for the thruput test.
 */
public class TransReceiver {
      protected static String clientHandle = "nat:12345";
  
    String serverAddress;
    XmlRpcClient client;
    String clientAddress;
    
    
    HashSet<Listener> listeners = new HashSet<Listener>();
    HashSet<Transmitter> transmitters = new HashSet<Transmitter>();
    private int npacket = 1000;
    int sleepTime = 30;

    class Transmitter implements Runnable {
        String ipAddr;
        int port;
        int count;
        DatagramSocket datagramSocket;

        public Transmitter(DatagramSocket socket , String ipAddr, int port, int count) {
            this.ipAddr = ipAddr;
            this.port = port;
            this.count = count;
            this.datagramSocket = socket;
        }

        public void run() {
            try {
                byte[] data = new byte[32];
                DatagramPacket datagramPacket = new DatagramPacket(data,
                        data.length, InetAddress.getByName(ipAddr), port);
                 for (int i = 0; i < npacket; i++) {
                   
                    Thread.sleep(sleepTime);
                    datagramSocket.send(datagramPacket);
                }
            } catch (Exception ex) {
                ex.printStackTrace();
            }

        }
   
    }
    
    class Listener implements Runnable {
        int counter;
        private DatagramSocket datagramSocket;
        private long time = -1;
        private long  delay = 0;

        public Listener(DatagramSocket datagramSocket) {
            this.datagramSocket = datagramSocket;
        }

        public void run() {
            byte[] data = new byte[32];
            DatagramPacket datagramPacket = new DatagramPacket(data,
                    data.length);
            while (true) {
                try {
                 
                    this.datagramSocket.receive(datagramPacket);
                    //if ( counter % 500 == 0 ) {
                    //    System.out.println("Counter = " + counter  );
                    //}
                    if ( time == -1 ) {
                        time = System.currentTimeMillis();
                    } else {
                        long newtime = System.currentTimeMillis();
                        int delta = ( int) ( newtime - time);
                        delay += (delta - sleepTime)*(delta - sleepTime);
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
    
    
    protected String createEvenSym() throws Exception {
        int count = 1;
        Object[] args = new Object[3];
        args[0] = clientHandle;
        args[1] = new Integer(count);
        args[2] = Symmitron.EVEN;
        
        Map retval = (Map) client.execute("sipXbridge.createSyms", args);
        if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " +
                    retval.get(Symmitron.ERROR_INFO));
        }
        Object[] syms = (Object[]) retval.get(Symmitron.SYM_SESSION);
        Map sym = (Map) syms[0];
        return (String)sym.get("id");
    }
    
    protected String createBridge() throws Exception {
        Object[] args = new Object[1];
        args[0] = this.clientHandle;
        Map retval = (Map) client.execute("sipXbridge.createBridge", args);
        if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " +
                    retval.get(Symmitron.ERROR_INFO));
        }
        return (String) retval.get(Symmitron.BRIDGE_ID);
    }
    
    protected void destroyBridge(String bridgeId) throws Exception {
        Object[] args = new Object[2];
        args[0] = this.clientHandle;
        args[1] = bridgeId;
        Map retval = (Map) client.execute("sipXbridge.destroyBridge",args);
        if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " +
                    retval.get(Symmitron.ERROR_INFO));
        }
    }
    
    protected String createOddSym() throws Exception {
        int count = 1;
        Object[] args = new Object[3];
        args[0] = clientHandle;
        args[1] = new Integer(count);
        args[2] = Symmitron.ODD;
        
        Map retval = (Map) client.execute("sipXbridge.createSyms", args);
        if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " +
                    retval.get(Symmitron.ERROR_INFO));
        }
        Object[] syms = (Object[]) retval.get(Symmitron.SYM_SESSION);
        return ( String ) syms[0];
    }
    
    protected void setRemoteEndpoint( String sym, int destinationPort) throws Exception {
        Object[] params = new Object[7];
        params[0] = clientHandle;
        params[1] = sym;
        params[2] = clientAddress;
        params[3] = new Integer(destinationPort);
        params[4] = new Integer(500);
        params[5] = "USE-EMPTY-PACKET";
        params[6] = "";
        Map retval = (Map)client.execute("sipXbridge.setDestination", params);
        if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " +
                    retval.get(Symmitron.ERROR_INFO));
        }

    }
    
    protected void setRemoteEndpointNoKeepAlive(String sym , int destinationPort)
        throws Exception {
        Object[] params = new Object[7];
        params[0] = clientHandle;
        params[1] = sym;
        params[2] = clientAddress;
        params[3] = new Integer(destinationPort);
        params[4] = new Integer(500);
        params[5] = "NONE";
        params[6] = "";
        Map retval = (Map)client.execute("sipXbridge.setDestination", params);
        if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " +
                    retval.get(Symmitron.ERROR_INFO));
        }

    }
    
    protected void setAutoDiscover( String sym) throws Exception {
        Object[] params = new Object[7];
        params[0] = clientHandle;
        params[1] = sym;
        params[2] = "";
        params[3] = 0;
        params[4] = new Integer(500);
        params[5] = "USE-EMPTY-PACKET";
        params[6] = "";
      
        Map retval = (Map)client.execute("sipXbridge.setDestination", params);
        if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " +
                    retval.get(Symmitron.ERROR_INFO));
        }

    }
    
    protected void addSym (  String bridge, String sym)  throws Exception  {
        Object[] params = new Object[3];
        params[0] = clientHandle;
        params[1] = bridge;
        params[2] = sym;
        Map retval = (Map) client.execute("sipXbridge.addSym",params);
        if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " +
                    retval.get(Symmitron.ERROR_INFO));
        }
    }
    
    protected void removeSym(String bridge, String sym) throws Exception    {
        Object[] parms = new Object[3];
        parms[0] = clientHandle;
        parms[1] = bridge;
        parms[2] = sym;
        Map retval = (Map) client.execute("sipXbridge.removeSym",parms);
        if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " +
                    retval.get(Symmitron.ERROR_INFO));
        }
        
    }
    
    /**
     * Get a Sym.
     */
    protected SymInterface getSym(String sym) throws Exception {
        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym;
        Map retval = (Map) client.execute("sipXbridge.getSym", args);
        if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " +
                    retval.get(Symmitron.ERROR_INFO));
        }
        SymImpl symImpl = new SymImpl();
        
        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = ( Map) symSession.get("receiver");
        if ( receiverSession != null && !receiverSession.isEmpty() ) {
            String ipAddr = (String) receiverSession.get("ipAddress");
            int port = (Integer) receiverSession.get("port");
            String id = (String)receiverSession.get("id");
       
            SymEndpointImpl receiverEndpoint  = new SymEndpointImpl();
            receiverEndpoint.setIpAddress(ipAddr);
            receiverEndpoint.setPort(port);
            receiverEndpoint.setId(id);
            symImpl.setReceiver(receiverEndpoint);
        }
        
        Map transmitterSession = (Map) symSession.get("transmitter");
        if ( transmitterSession != null && ! transmitterSession.isEmpty() ) { 
            String ipAddr = (String) transmitterSession.get("ipAddress");
            
           
            int port = (Integer) transmitterSession.get("port");
            String id = (String)transmitterSession.get("id");
            
            SymEndpointImpl transmitterEndpoint  = new SymEndpointImpl();
            transmitterEndpoint.setIpAddress(ipAddr);
            transmitterEndpoint.setPort(port);
            transmitterEndpoint.setId(id);
            symImpl.setTransmitter(transmitterEndpoint);
        }
        return symImpl;
    }
    
   public void pauseBridge(String bridge) throws Exception  {
       Object[] args = new Object[2];
       args[0] = clientHandle;
       args[1] = bridge;
       Map retval = (Map) client.execute("sipXbridge.pauseBridge",args);
       if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
           throw new Exception("Error in processing request " +
                   retval.get(Symmitron.ERROR_INFO));
       }
   }
    
   public void resumeBridge(String bridge) throws Exception  {
       Object[] args = new Object[2];
       args[0] = clientHandle;
       args[1] = bridge;
       Map retval = (Map) client.execute("sipXbridge.resumeBridge",args);
       if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
           throw new Exception("Error in processing request " +
                   retval.get(Symmitron.ERROR_INFO));
       }
   }
   
   public void startBridge(String bridge) throws Exception  {
       Object[] args = new Object[2];
       args[0] = clientHandle;
       args[1] = bridge;
       Map retval = (Map) client.execute("sipXbridge.startBridge", args);
       if ( retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
           throw new Exception("Error in processing request " +
                   retval.get(Symmitron.ERROR_INFO));
       }
   }
   
   
   
    protected void signIn() throws Exception {
        String[] myHandle = new String[1] ;
        myHandle[0] = clientHandle;
        Map retval = (Map) client.execute("sipXbridge.signIn",
                (Object[]) myHandle);
    }





    public void destroySym(String sym) throws Exception {
       Object[] args = new Object[2];
       args[0] = clientHandle;
       args[1] = sym;
       Map retval = (Map) client.execute("sipXbridge.destroySym",args);
        
    }
    public TransReceiver(boolean server, String serverAddress, String clientAddress, int port) throws Exception {
        this.serverAddress = serverAddress;
        this.clientAddress = clientAddress;
        if ( !server ) {
            XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();

            System.out.println("Client URL = " + "http://" + serverAddress + ":"
                    + port);
            config
            .setServerURL(new URL("http://" + serverAddress + ":"
                    + port));

            client = new XmlRpcClient();

            client.setConfig(config);
        }
    }
    
    public void testThruput(int count) throws Exception {
        int destinationPort1 = 29000;
        int destinationPort2 = 40000;
        
        for (int i = 0; i < count; i++ ) {
            String bridge = createBridge();
            String sym = createEvenSym();
            SymInterface symInterface = getSym(sym);
            SymEndpointInterface symEndpoint = symInterface.getReceiver();
            int port1 =  symEndpoint.getPort();
            System.out.println("port1 = " + port1);
            destinationPort1 += i;
            System.out.println("dest port " + destinationPort1);
            DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1,
                    InetAddress.getByName(clientAddress));
            Listener listener1 = new Listener( datagramSocket1);
            this.listeners.add(listener1);
            setRemoteEndpointNoKeepAlive(sym, destinationPort1);
            addSym(bridge, sym);
            
            
          
            sym = createEvenSym();
            symInterface = getSym(sym);
            symEndpoint = symInterface.getReceiver();
            int port2 =  symEndpoint.getPort();
            System.out.println("port2 = " + port2);
            destinationPort2 += i;
            DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2,
                    InetAddress.getByName(clientAddress));
            Listener listener2 = new Listener( datagramSocket2);
            this.listeners.add(listener2);
            setRemoteEndpointNoKeepAlive(sym, destinationPort2);
            addSym(bridge, sym);
            
            startBridge(bridge);
            
            Transmitter transmitter1 = new Transmitter(datagramSocket2, serverAddress,port1,1000);
            Transmitter transmitter2 = new Transmitter(datagramSocket1, serverAddress,port2,1000);
            transmitters.add(transmitter1);
            transmitters.add(transmitter2);
            
            
        }
        for ( Listener listener : this.listeners) {
            new Thread(listener).start();
        }
        for ( Transmitter transmitter : this.transmitters) {
            new Thread(transmitter).start();
            Thread.sleep(100);
        }
        
        Thread.sleep(100000);
        
       
        for ( Listener listener : this.listeners) {
            listener.datagramSocket.close();
        }
        for ( Listener listener : this.listeners) {
            TestCase.assertTrue ( "Count should be " + npacket ,  listener.counter >= npacket);
        }
        double jitter = 0;
        for ( Listener listener : this.listeners) {
            
            long avgDelay = listener.delay / listener.counter;
            double rmsDelay = Math.sqrt((double) avgDelay);
            jitter += rmsDelay; 
        }
        System.out.println("RMS jitter = " + sleepTime + " " +
                jitter / listeners.size());
     }
    
    
    
    public static void main(String[] args) throws Exception {
        TransReceiver transceiver;
        boolean isServer = args[0].equals("true");
        String  clientAddress = args[1];
        String  serverAddress = args[2];
        int port = Integer.parseInt(args[3]);
        
        if ( isServer ) {
            transceiver = new TransReceiver( true,clientAddress,serverAddress,port);
        } else {
            transceiver = new TransReceiver(false,clientAddress,serverAddress,port);
        }
        
        if ( !isServer) {
            int count  = Integer.parseInt(args[4]);
            
           transceiver.testThruput(count);
        }
    }
    
    

}
