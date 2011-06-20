/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.net.DatagramPacket;
import java.util.PriorityQueue;
import java.util.Queue;

import junit.framework.TestCase;

public class RtpPacketTest extends TestCase
{

   void setSequenceNumber(DatagramPacket packet, long sequenceNumber)
   {
      byte[] octets = packet.getData() ;
      // Install sequence number into RTP header in big endian order
      octets[ 2] = (byte)((sequenceNumber & 0xFF00) >> 8) ;
      octets[ 3] = (byte)((sequenceNumber & 0x00FF)) ;
   }

   public void testGetSequenceNumber()
   {
      byte[] octets = new byte[10] ;
      DatagramPacket packet = new DatagramPacket(octets,10) ;

      long sequenceNumber = 42L ;
      setSequenceNumber(packet, sequenceNumber) ;
      RtpPacket rtp = new RtpPacket(packet) ;
      assertEquals(sequenceNumber, rtp.getSequenceNumber()) ;

      sequenceNumber = 65535L ;
      setSequenceNumber(packet, sequenceNumber) ;
      rtp = new RtpPacket(packet) ;
      assertEquals(sequenceNumber, rtp.getSequenceNumber()) ;
   }


   public void testEquals()
   {
      byte[] octets = new byte[10] ;
      DatagramPacket p = new DatagramPacket(octets,10) ;
      RtpPacket a, b;

      long sequenceNumber = 0L ;
      setSequenceNumber(p, sequenceNumber) ;
      a = new RtpPacket(p) ;
      boolean e = a.equals(a);
      assertTrue(e);

      sequenceNumber = 42L ;
      setSequenceNumber(p, sequenceNumber) ;
      b = new RtpPacket(p) ;

      e = a.equals(b);
      assertFalse(e) ;

      sequenceNumber = 0L ;
      setSequenceNumber(p, sequenceNumber) ;
      b = new RtpPacket(p) ;

      e = a.equals(b);
      assertTrue(e) ;

   }

   public void testCompareTo()
   {
      byte[] octets = new byte[10] ;
      DatagramPacket p = new DatagramPacket(octets,10) ;
      RtpPacket a, b, c ;

      long sequenceNumber = 42L ;
      setSequenceNumber(p, sequenceNumber) ;
      a = new RtpPacket(p) ;

      sequenceNumber = 43L ;
      setSequenceNumber(p, sequenceNumber) ;
      b = new RtpPacket(p) ;

      assertEquals(0, a.compareTo(a)) ; // Better match itself!
      assertEquals(-1, a.compareTo(b)) ; // a (42) is less than b (43)
      assertEquals(1, b.compareTo(a)) ; // b (43) is greater than a (42)


      sequenceNumber = 0L ;
      setSequenceNumber(p, sequenceNumber) ;
      a = new RtpPacket(p) ;

      sequenceNumber = 1L ;
      setSequenceNumber(p, sequenceNumber) ;
      b = new RtpPacket(p) ;

      assertEquals(-1, a.compareTo(b)) ; // a (0) is less than b (1)
      assertEquals(1, b.compareTo(a)) ; // b (1) is greater than a (0)


      sequenceNumber = 65535L ;
      setSequenceNumber(p, sequenceNumber) ;
      a = new RtpPacket(p) ;

      sequenceNumber = 0L ;
      setSequenceNumber(p, sequenceNumber) ;
      b = new RtpPacket(p) ;

      assertEquals(-1, a.compareTo(b)) ; // a (65535) is less than b (0)
      assertEquals(1, b.compareTo(a)) ; // b (0) is greater than a (65535)

      for(long l=0; l<65536; l++)
      {
         setSequenceNumber(p, l) ;
         a = new RtpPacket(p) ;

         setSequenceNumber(p, l+1) ;
         b = new RtpPacket(p) ;

         setSequenceNumber(p, l+10) ;
         c = new RtpPacket(p) ;

         assertEquals(-1, a.compareTo(b)) ;
         assertEquals(-1, a.compareTo(c)) ;
         assertEquals(1, b.compareTo(a)) ;
         assertEquals(-1, b.compareTo(c)) ;
      }
   }

public void testQueue() {
      byte[] octets = new byte[10] ;
      DatagramPacket p = new DatagramPacket(octets,10) ;
      RtpPacket a, b, c, d ;

      long sequenceNumber = 42L ;
      setSequenceNumber(p, sequenceNumber) ;
      a = new RtpPacket(p) ;

      sequenceNumber = 43L ;
      setSequenceNumber(p, sequenceNumber) ;
      b = new RtpPacket(p) ;

      sequenceNumber = 42L ;
      setSequenceNumber(p, sequenceNumber) ;
      c = new RtpPacket(p) ;

      sequenceNumber = 44L ;
      setSequenceNumber(p, sequenceNumber) ;
      d = new RtpPacket(p) ;

      Queue<RtpPacket> q = new PriorityQueue<RtpPacket>();
      q.add(a);
      q.add(b);

      assertTrue(q.contains(a));
      assertTrue(q.contains(b));
      assertTrue(q.contains(c));
      assertFalse(q.contains(d));
      assertTrue(q.remove(c));
      assertFalse(q.remove(a));
   }
}
