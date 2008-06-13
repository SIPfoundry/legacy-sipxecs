package org.sipfoundry.sipxbridge;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.URL;
import java.util.HashSet;
import java.util.Properties;

import junit.framework.TestCase;

import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.sipxbridge.symmitron.SymEndpointInterface;
import org.sipfoundry.sipxbridge.symmitron.SymInterface;

public class SymmitronThruputTest extends AbstractSymmitronTestCase {
    
    HashSet<Listener> listeners = new HashSet<Listener>();
    HashSet<Transmitter> transmitters = new HashSet<Transmitter>();
    private int npacket = 1000;
    int sleepTime = 160;

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
    
    public void setUp() throws Exception {
        super.setUp();
        super.start();
        super.signIn();
        
    }
   
    
    public void testThruput10() throws Exception {
       int destinationPort1 = 29000;
       int destinationPort2 = 40000;
       
       for (int i = 0; i < 50; i++ ) {
           String bridge = super.createBridge();
           String sym = super.createEvenSym();
           SymInterface symInterface = super.getSym(sym);
           SymEndpointInterface symEndpoint = symInterface.getReceiver();
           int port1 =  symEndpoint.getPort();
           System.out.println("port1 = " + port1);
           destinationPort1 += i;
           System.out.println("dest port " + destinationPort1);
           DatagramSocket datagramSocket1 = new DatagramSocket(destinationPort1,
                   InetAddress.getByName(serverAddress));
           Listener listener1 = new Listener( datagramSocket1);
           this.listeners.add(listener1);
           super.setRemoteEndpointNoKeepAlive(sym, destinationPort1);
           super.addSym(bridge, sym);
           
           
         
           sym = super.createEvenSym();
           symInterface = super.getSym(sym);
           symEndpoint = symInterface.getReceiver();
           int port2 =  symEndpoint.getPort();
           System.out.println("port2 = " + port2);
           destinationPort2 += i;
           DatagramSocket datagramSocket2 = new DatagramSocket(destinationPort2,
                   InetAddress.getByName(serverAddress));
           Listener listener2 = new Listener( datagramSocket2);
           this.listeners.add(listener2);
           super.setRemoteEndpointNoKeepAlive(sym, destinationPort2);
           super.addSym(bridge, sym);
           
           super.startBridge(bridge);
           
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
       
       Thread.sleep(200000);
       
      
       for ( Listener listener : this.listeners) {
           listener.datagramSocket.close();
       }
       for ( Listener listener : this.listeners) {
           assertTrue ( "Count should be " + npacket ,  listener.counter >= npacket);
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

}
