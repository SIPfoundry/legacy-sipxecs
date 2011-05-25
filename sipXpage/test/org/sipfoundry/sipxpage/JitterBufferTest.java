/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.net.DatagramPacket;

import junit.framework.TestCase;

public class JitterBufferTest extends TestCase
{

   private DatagramPacket generate(int sequenceNumber)
   {
      byte[] octets = new byte[4] ;
      // Install sequence number into RTP header in big endian order
      octets[ 2] = (byte)((sequenceNumber & 0xFF00) >> 8) ;
      octets[ 3] = (byte)((sequenceNumber & 0x00FF)) ;
      return new DatagramPacket(octets,4) ;
   }

   public void testAdd()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      DatagramPacket p ;

      p = j.removeDatagram() ;
      if (p != null)
      {
         fail("Empty JitterBuffer did not return null");
      }

      j.add(generate(0)) ;
      j.add(generate(1)) ;
      j.add(generate(2)) ;

      assertEquals(3, j.size()) ;
   }

   public void testAddDuplicate()
   {
      JitterBuffer j = new JitterBuffer(5) ;

      j.add(generate(0)) ;
      j.add(generate(1)) ;
      j.add(generate(2)) ;
      j.add(generate(0)) ;

      assertEquals(3, j.size()) ;
   }

   public void testAddDuplicate2()
   {
      JitterBuffer j = new JitterBuffer(5) ;

      j.add(generate(0)) ;
      j.add(generate(1)) ;
      j.removeRtpPacket() ;
      j.removeRtpPacket() ;
      j.add(generate(0)) ; // Should be treated as "late", not "dup"

      assertEquals(0, j.size()) ;
   }

   public void testAddDuplicate3()
   {
      JitterBuffer j = new JitterBuffer(5) ;

      j.add(generate(0)) ;
      j.add(generate(1)) ;
      j.removeRtpPacket() ;
      j.add(generate(0)) ;
      j.removeRtpPacket() ;

      assertEquals(0, j.size()) ;
   }

   public void testRemoveInOrder()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      RtpPacket r ;

      // Install three packets in order.
      j.add(generate(0)) ;
      j.add(generate(1)) ;
      j.add(generate(2)) ;

      // Check they come out in order
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(0, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(1, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(2, r.getSequenceNumber()) ;

   }

   public void testRemoveReverseOrder()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      RtpPacket r ;

      // Install three packets in reverse order.
      j.add(generate(2)) ;
      j.add(generate(1)) ;
      j.add(generate(0)) ;

      // Check they come out in order
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(0, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(1, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(2, r.getSequenceNumber()) ;
   }

   public void testRemoveDropped()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      RtpPacket r ;

      // Install three packets, one missing
      j.add(generate(0)) ;
      j.add(generate(1)) ;
      j.add(generate(3)) ;

      // Check the first two come out in order
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(0, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(1, r.getSequenceNumber()) ;

      // The third call should return null
      r = j.removeRtpPacket() ;
      assertNull(r) ;

      // The fourth call should return the last packet
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(3, r.getSequenceNumber()) ;
   }

   public void testRemoveLate()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      RtpPacket r ;

      // Install three packets, one missing
      j.add(generate(0)) ;
      j.add(generate(1)) ;
      j.add(generate(3)) ;

      // Check the first two come out in order
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(0, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(1, r.getSequenceNumber()) ;

      // The third call should return null
      r = j.removeRtpPacket() ;
      assertNull(r) ;

      // Now sequence 2 arrives:
      j.add(generate(2)) ;

      // The fourth call should return 2
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(2, r.getSequenceNumber()) ;

      // The fifth call should return 3
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(3, r.getSequenceNumber()) ;
   }

   public void testRemoveLateDropped()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      RtpPacket r ;

      // Install three packets, one missing
      j.add(generate(0)) ;
      j.add(generate(1)) ;
      j.add(generate(3)) ;

      // Check the first two come out in order
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(0, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(1, r.getSequenceNumber()) ;

      // The third call should return null
      r = j.removeRtpPacket() ;
      assertNull(r) ;

      // Now sequence 4 arrives:
      j.add(generate(4)) ;

      // The fourth call should return 3
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(3, r.getSequenceNumber()) ;

      // The fifth call should return 4
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(4, r.getSequenceNumber()) ;
   }

   public void testRemoveTooLateDropped()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      RtpPacket r ;

      // Install three packets, one missing
      j.add(generate(0)) ;
      j.add(generate(1)) ;
      j.add(generate(3)) ;

      // Check the first two come out in order
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(0, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(1, r.getSequenceNumber()) ;

      // The third call should return null
      r = j.removeRtpPacket() ;
      assertNull(r) ;

      // Now sequence 65535 arrives (it should fail)
      assertFalse(j.add(generate(65535))) ;

      // The fourth call should return 3
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(3, r.getSequenceNumber()) ;

      // The fifth call should return null
      r = j.removeRtpPacket() ;
      assertNull(r) ;
   }

   public void testRemoveInOrderWrap()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      RtpPacket r ;

      // Install 4 packets in order.
      j.add(generate(65534)) ;
      j.add(generate(65535)) ;
      j.add(generate(0)) ;
      j.add(generate(1)) ;

      // Check they come out in order
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(65534, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(65535, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(0, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(1, r.getSequenceNumber()) ;
   }

   public void testRemoveInOrderBoundry()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      RtpPacket r ;

      // Install 4 packets in odd order.
      j.add(generate(32768)) ;
      j.add(generate(32766)) ;
      j.add(generate(32769)) ;
      j.add(generate(32767)) ;

      // Check they come out in order
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(32766, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(32767, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(32768, r.getSequenceNumber()) ;
      r = j.removeRtpPacket() ;
      assertNotNull(r) ;
      assertEquals(32769, r.getSequenceNumber()) ;
   }

   public void testRemoveMax()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      RtpPacket r ;

      // Install 10 packets in order.
      for(int i=1; i<=10; i++)
      {
         j.add(generate(i)) ;
      }

      for(int i=6;i<=10;i++)
      {
         // Check they come out in order, starting with the 6th
         r = j.removeRtpPacket() ;
         assertNotNull(r) ;
         assertEquals(i, r.getSequenceNumber()) ;
      }

      // And the last is null
      r = j.removeRtpPacket() ;
      assertNull(r) ;
   }

   public void testRemoveMany()
   {
      JitterBuffer j = new JitterBuffer(5) ;
      RtpPacket r ;

      for(int i=0; i<65535*2; i++)
      {
         j.add(generate(i)) ;
         r = j.removeRtpPacket() ;
         assertNotNull("Got null on packet "+i, r) ;
         assertEquals(i&0xFFFF, r.getSequenceNumber()) ;
      }
   }
}
