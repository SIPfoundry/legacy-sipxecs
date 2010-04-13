/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketException;

import junit.framework.TestCase;

public class RtpForkTest extends TestCase implements LegListener
{

   public boolean onEvent(LegEvent event)
   {
      return false;
   }

   public class TestSocket extends PacketSocket
   {
      public String sent ;
      long seqs[] = new long[5] ;
      int n = 0 ;


      public TestSocket(DatagramSocket socket)
      {
         super(socket);
      }

      public TestSocket(int port) throws SocketException
      {
         super(port);
      }


      public void reset()
      {
         sent = "" ;
         n = 0 ;
      }

      @Override
      public InetSocketAddress getInetSocketAddress()
      {
         return super.getInetSocketAddress();
      }

      @Override
      public DatagramPacket read()
      {
         return super.read();
      }

      @Override
      public void send(SocketAddress address, DatagramPacket packet)
      {
         RtpPacket rtp = new RtpPacket(packet) ;
         sent += "Sent to "+address.toString()+": "+packet.getData()[0]+"%n" ;

         seqs[n++] = rtp.getSequenceNumber() ;
      }

   }


   public void testDestinations()
   {
      PacketSocket s = null;
      try
      {
         s = new TestSocket(0);
         RtpFork r = new RtpFork(s,20, this) ;
         assertEquals(0, r.destinations.size()) ; // Should start empty
         SocketAddress a = new InetSocketAddress(42) ;
         r.addDestination(a) ;
         assertEquals(1, r.destinations.size()) ; // Should be one
         r.removeDestination(a) ;
         assertEquals(0, r.destinations.size()) ; // Should be zero
         SocketAddress b = new InetSocketAddress(43) ;
         r.addDestination(a) ;
         r.addDestination(b) ;
         assertEquals(2, r.destinations.size()) ; // Should be two
         r.removeAllDestinations() ;
         assertEquals(0, r.destinations.size()) ; // Should be zero
      } catch (SocketException e)
      {
         // TODO Auto-generated catch block
         e.printStackTrace();
      }
   }


   public void testSend()
   {
      TestSocket t = null ;
      PacketSocket s = null;
      try
      {
         t = new TestSocket(0);
         s = t ;
         RtpFork r = new RtpFork(s, 20, this) ;
         assertEquals(0, r.destinations.size()) ; // Should start empty
         SocketAddress a = new InetSocketAddress(42) ;
         byte[] oa = new byte[10] ;
         DatagramPacket packet = new DatagramPacket(oa, oa.length) ;
         oa[0] = 0 ;
         SocketAddress b = new InetSocketAddress(88) ;

         t.reset() ;
         r.send(packet) ;
         assertEquals("", t.sent) ;

         r.addDestination(a) ;
         t.reset() ;
         r.send(packet) ;
         assertEquals("Sent to 0.0.0.0/0.0.0.0:42: 0%n", t.sent) ;

         r.addDestination(b) ;
         t.reset() ;
         r.send(packet) ;
         assertEquals("Sent to 0.0.0.0/0.0.0.0:42: 0%nSent to 0.0.0.0/0.0.0.0:88: 0%n", t.sent) ;

         r.removeDestination(a);
         t.reset() ;
         r.send(packet) ;
         assertEquals("Sent to 0.0.0.0/0.0.0.0:88: 0%n", t.sent) ;

      } catch (SocketException e)
      {
         fail(e.toString()) ;
      }
   }

   public void testRtpSend()
   {
      TestSocket t = null ;
      PacketSocket s = null;
      try
      {
         t = new TestSocket(0);
         s = t ;
         RtpFork r = new RtpFork(s, 20, this) ;
         assertEquals(0, r.destinations.size()) ; // Should start empty
         SocketAddress a = new InetSocketAddress(42) ;
         byte[] oa = new byte[100] ;
         DatagramPacket packet = new DatagramPacket(oa, oa.length) ;
         oa[0] = 0 ;
         SocketAddress b = new InetSocketAddress(88) ;

         t.reset() ;
         r.rtpSend(packet) ; // seq 43
         assertEquals(0, t.n) ;

         r.addDestination(a) ;
         t.reset() ;
         r.rtpSend(packet) ; // seq 44
         assertEquals("Sent to 0.0.0.0/0.0.0.0:42: 0%n", t.sent) ;
         assertEquals(44L, t.seqs[0]) ;

         r.addDestination(b) ;
         t.reset() ;
         r.rtpSend(packet) ; // seq 45
         assertEquals("Sent to 0.0.0.0/0.0.0.0:42: 0%nSent to 0.0.0.0/0.0.0.0:88: 0%n", t.sent) ;
         assertEquals(45L, t.seqs[0]) ;

         r.removeDestination(a);
         t.reset() ;
         r.rtpSend(packet) ; // seq 46
         r.rtpSend(packet) ; // seq 47
         assertEquals("Sent to 0.0.0.0/0.0.0.0:88: 0%nSent to 0.0.0.0/0.0.0.0:88: 0%n", t.sent) ;
         assertEquals(46L, t.seqs[0]) ;
         assertEquals(47L, t.seqs[1]) ;

      } catch (SocketException e)
      {
         fail(e.toString()) ;
      }
   }

   public void testStartLocalAudio()
   {
      // TODO fail("Not yet implemented");
   }

   public void testStopLocalAudio()
   {
      // TODO fail("Not yet implemented");
   }

}
