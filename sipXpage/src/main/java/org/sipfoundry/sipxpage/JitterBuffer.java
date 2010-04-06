/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.net.DatagramPacket;
import java.util.PriorityQueue;

import org.apache.log4j.Logger;


/**
 * A sequence number based Jitter buffer.
 *
 * Install packets in the order they arrive from the network.
 * Remove packets in sequence number order.
 * <p>
 * Returns the next available packet if it matches the next natural
 * sequence number.  Returns null (meaning no more packets) if the
 * buffer is empty, or if the last packet in the buffer is NOT less than
 * the next natural sequence number (implying a lost packet).  By not
 * returning this last packet, it gives a chance for that packet to show
 * up late, and thus be inserted in the proper order.  If the next time
 * a packet is requested the "correct" packet still is not available,
 * then it returns the next best packet.
 * <p>
 * This will "delay" a packet by at most one time interval if a missing
 * packet is detected, giving it one time interval to show up before
 * skipping it.
 *
 * @author Woof!
 *
 */
public class JitterBuffer
{
   static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxpage") ;
   long lastSequenceNumber = -1L ;
   PriorityQueue<RtpPacket> queue ;
   int maxDepth ;
   int empty = 0 ;
   boolean oneLeft = false ;

   public JitterBuffer(int maxDepth)
   {
      this.maxDepth = maxDepth ;
      queue = new PriorityQueue<RtpPacket>(maxDepth) ;
   }

   /**
    *
    * @param packet The DatagramPacket (assumed to be RTP) to put into the Jitter Buffer
    * @return true for success, false for failure
    */
   public boolean add(DatagramPacket packet)
   {
      // Wrap the datagram with RtpPacket
      RtpPacket rtpPacket = new RtpPacket(packet) ;
      return add(rtpPacket) ;
   }

   /**
    *
    * @param rtpPacket The RtpPacket to put into the Jitter Buffer
    * @return true for success, false for failure
    */
   public boolean add(RtpPacket rtpPacket)
   {
      // If the sequence number of the new packet is BEFORE the
      // last packet removed, then it is too late to use now.  Drop it.
      if (lastSequenceNumber >= 0)
      {
         if (RtpPacket.compareSequence(rtpPacket.getSequenceNumber(), lastSequenceNumber) <= 0)
         {
            LOG.debug(String.format("JitterBuffer::add sequence too late "+rtpPacket.getSequenceNumber())) ;
            return false ;
         }
      }

      // If this sequence number already exists, it's a dup.  Drop it.
      // (actually, remove the one in the queue, and add this one again.)
      if (queue.remove(rtpPacket))
      {
         LOG.debug(String.format("JitterBuffer::add sequence duplication "+rtpPacket.getSequenceNumber())) ;
         return queue.add(rtpPacket) ;
      }

      // If the queue is full, toss the last packet to make room.
      if (queue.size() >= maxDepth)
      {
         removeRtpPacket() ;
      }

//      LOG.debug(String.format("sequence added "+rtpPacket.getSequenceNumber())) ;
      return queue.add(rtpPacket) ;
   }

   /**
    * Remove the next available RtpPacket from the Jitter Buffer, in sequence number
    * order.
    * @return The RtpPacket with the lowest sequence number in the buffer.  Returns null
    * if there is no available RtpPacket to return.  Will allow the buffer to run "dry" (empty) if the
    * last packet returned had the expected next sequence number.  Otherwise, it will try to retain
    * one last packet in the buffer in order to give time for an older packet to show up.  If
    * no older packet shows up by the time removeRtpPacket() is called again, the final packet
    * is returned.
    */
   public RtpPacket removeRtpPacket()
   {
      RtpPacket rtpPacket = queue.peek();
      boolean sendIt = true ;
      @SuppressWarnings("unused")
      String msg = "";

      // No packets in the queue.
      if (rtpPacket == null)
      {
    	 empty++ ;
    	 if (empty > maxDepth)
    	 {
    		 // After enough time, reset the buffer to accept a new sequence set
    		 lastSequenceNumber = -1L ;
    	 }
         return null;
      }

      long nextSequenceNumber = (lastSequenceNumber + 1) & 0xFFFF;
      long sequenceNumber = rtpPacket.getSequenceNumber();

      for(;;)
      {
         if (lastSequenceNumber < 0) // -1 indicates not yet set
         {
            msg = "First sequence" ;
            break ;
         }

         // If the next packet in the queue happens to be the next natural
         // packet to send, then send it.  Even if it empties the queue.
         if (sequenceNumber == nextSequenceNumber)
         {
            msg = "Natural sequence" ;
            break ;
         }

         // If there is more than one packet left in the queue, send the lowest one.
         if(queue.size() > 1)
         {
            msg = "Size sequence" ;
            break ;
         }

         // If this is the second time with the same packet left, send it on.
         if (oneLeft)
         {
            oneLeft = false ;
            msg = "oneLeft sequence" ;
            break ;
         }

         sendIt = false ;
         break ;
      }

      if (sendIt)
      {
         // Save the last sequence number for later
         lastSequenceNumber = sequenceNumber ;
//         LOG.debug(String.format("JitterBuffer::removeRtpPacket %s %d depth %d%n", msg, sequenceNumber, queue.size())) ;
         return queue.remove();
      }
      else
      {
         // Leave the last packet in the queue
//         LOG.debug(String.format("JitterBuffer::removeRtpPacket oneLeft not sent")) ;
         oneLeft = true ;
         return null ;
      }
   }

   /**
    * Same as removeRtpPacket but unwraps the RtpPacket to return the DatagramPacket
    * @return see @link removeRtpPacket
    */
   public DatagramPacket removeDatagram()
   {
      RtpPacket rtpPacket = removeRtpPacket() ;
      return rtpPacket == null ? null : rtpPacket.getDatagram() ;
   }

   public int size()
   {
      return queue.size();
   }
}
