/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.net.DatagramPacket;



/**
 * Wraps an RTP Datagram with a class that supports Comparable based on sequence number.
 * This allows a Jitter Buffer to sort on sequence number order.
 *
 * @author Woof!
 *
 */
public class RtpPacket implements Comparable<RtpPacket>
{
   long sequenceNumber ;
   DatagramPacket packet ;

   public RtpPacket(DatagramPacket packet)
   {
      byte [] octets = packet.getData() ;
      // byte is SIGNED (sheesh).  So mask off lower bits when converting to long
      sequenceNumber = ((octets[2] & 0xFF) << 8) + (octets[3] & 0xFF) ;
      this.packet = packet ;
   }

   public long getSequenceNumber()
   {
      return sequenceNumber ;
   }

   public DatagramPacket getDatagram()
   {
      return packet ;
   }

   public int hashCode()
   {
	   return (int)sequenceNumber;
   }

   public boolean equals(Object a)
   {
	   try {
		   RtpPacket r = (RtpPacket)a;
		   return r.sequenceNumber == sequenceNumber;
	   } catch (Throwable t) {
		   // Anything goes wrong (most likely Cast Class Exception)
		   return false ;
	   }
   }

   public int compareTo(RtpPacket a)
   {
      return RtpPacket.compareSequence(sequenceNumber, a.getSequenceNumber()) ;
   }

   public static int compareSequence(long a, long b)
   {

      // Perform a circular 16 bit compare.
      // If the distance between the two numbers is larger than 32767,
      // and the numbers are larger than 32768, subtract 65536
      // Thus, 65535 compares less than 0, but greater than 65534
      // This handles the 65535->0 wrap around case correctly
      long dist = java.lang.Math.abs(a - b) ;
      if (dist > 32767)
      {
         if (a > 32768L)
         {
            a-= 65536 ;
         }
         if (b > 32768L)
         {
            b -= 65536 ;
         }
      }
      Long lA = a ;
      return lA.compareTo(b) ;
   }
}
