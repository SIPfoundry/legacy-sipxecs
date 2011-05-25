/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.SocketAddress;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

import org.apache.log4j.Logger;



/**
 * Accept inbound RTP packets, and forward the RTP packets to zero or more destinations.
 * This enables a "paging service" where one phone's RTP is "broadcast" to many other phones.
 *
 * Destinations may be added and removed "on the fly" without locks.
 *
 * Call the {@link #beat beat} method at a fixed rate periodically (once every 20 mS is ideal).  One
 * way to do that is with a thread service ({@code beat} is called by {@link run run}):
 *
 * <pre>
 * rtpFork = new RtpFork(socket)
 * service = new ScheduledThreadPoolExecutor(1) ;
 * service.scheduleAtFixedRate(rtpFork, 0, 20, TimeUnit.MILLISECONDS);
 * </pre>
 *
 * Note that RTCP packets are ignored (and may generate ICMP packets as there is no socket open
 * to eat them)
 *
 * The listener gets the following events:
 *    "localAudio end"  when the audio specified by setLocalAudio() ends.
 *
 * @author Woof!
 *
 */
public class RtpFork implements Runnable
{
   static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxpage") ;
   int rhythm ;            // Time (in mS) between beats
   PacketSocket socket ;
   ArrayList<SocketAddress> destinations ;
   DatagramPacket nextAudioPacket = null ;  // Packet held until next time slice
   long sequenceNumber ;   // The last RTP sequence number sent
   long timestamp ;        // The last RTP timestamp sent ;
   long firstStamp ;
   long firstTime = 0 ;
   int payloadSize ;        // The number of audio samples in an RTP packet
   byte [] rtpOutBuf ;
   DatagramPacket rtpOutPacket ;
   JitterBuffer jitterBuff ;
   AudioInputStream ulawStream = null ; // A local uLaw stream of bytes to packetize
   LegListener legListener ;

   ScheduledExecutorService service ;

   public static final int BYTES_PER_MS = 8;   // 8 bytes per mS at 8000 samples/second uLaw

   /**
    * @param socket The socket on which to recieve RTP packets.
    * @param rhythm The packitization rhythm (nominally 20 mS)
    * @param legListener The listener to send events to
    */
   public RtpFork(PacketSocket socket, int rhythm, LegListener legListener)
   {
      this.socket = socket ;
      this.rhythm = rhythm ;
      this.legListener = legListener ;
      destinations = new ArrayList<SocketAddress>(50) ;
      sequenceNumber = 42L ;
      timestamp = 0L ;
      payloadSize = rhythm * BYTES_PER_MS ;
      rtpOutBuf = new byte[12+payloadSize] ; // 12 byte RTP header + payload
      rtpOutPacket = new DatagramPacket(rtpOutBuf, rtpOutBuf.length) ;
      jitterBuff = new JitterBuffer(5) ;
   }

   /**
    * @param packet The packet to send to each destination
    */
   void send(DatagramPacket packet)
   {
      for (Iterator iter = destinations.iterator(); iter.hasNext();)
      {
         SocketAddress address = (SocketAddress) iter.next();
         //         System.out.printf("Sending %d to %s%n", packet.getLength(), address.toString()) ;
         socket.send(address, packet) ;
      }
   }

   static
   void dumpBytes(byte [] bytes, int length)
   {
     if (length > bytes.length)
     {
   	  length = bytes.length ;
     }
      for(int i=0; i<length;)
      {
         if (i % 16 == 0)
         {
            System.out.printf("%04d:", i) ;
         }
         int x = i ;
         for(int j=0; j<16; j++)
         {
            if (x < length)
            {
               System.out.printf(" %02x", bytes[x++]) ;
            }
            else
            {
               System.out.printf("   ") ;
            }
         }
         System.out.printf("  ") ;
         for(int j=0; j<16; j++)
         {
            byte b = ' ' ;
            if (i < length)
            {
               b = bytes[i++] ;
            }
            if (b < ' ' || b > 127)
            {
               b = '.' ;
            }
            System.out.printf("%c", b) ;
         }
         System.out.printf("%n") ;
      }
      System.out.printf("%n") ;
   }

   void rtpSend(DatagramPacket origRtp)
      {
         byte [] octets = origRtp.getData() ;

         // dumpBytes(octets, 12) ;
         sequenceNumber = ++sequenceNumber & 0xFFFF ;  // Increment and wrap at 16 bits

         // Install sequence number into RTP header in big endian order
         octets[ 2] = (byte)((sequenceNumber & 0xFF00) >> 8) ;
         octets[ 3] = (byte)((sequenceNumber & 0x00FF)) ;

         // Calculate payload length
         int numSamples = origRtp.getLength() - 12 ;  // 12 bytes of header, minimum
         if ((octets[0] & 0x20) != 0)                 // Padding bit set
         {
            // Get the padding size from the last byte
            int paddingSize = octets[origRtp.getLength()-1] ;
            numSamples -= paddingSize ;               // remove any padding
         }
         int csrcCount = octets[0] & 0x0F ;
         numSamples -= csrcCount*4 ;                  // and CSRC identifiers

         // Increment the timestamp by the payload length
         // (assumes one byte per sample codec, like PCMU)
         timestamp = (timestamp + numSamples) & 0xFFFFFFFF ; // Increment and wrap at 32 bits

         // Install into RTP header in big endian order
         octets[ 4] = (byte)((timestamp & 0xFF000000) >> 24) ;
         octets[ 5] = (byte)((timestamp & 0x00FF0000) >> 16) ;
         octets[ 6] = (byte)((timestamp & 0x0000FF00) >>  8) ;
         octets[ 7] = (byte)((timestamp & 0x000000FF)) ;

         // Install ssrc into RTP header in big endian order
         octets[ 8] = 'W' ;
         octets[ 9] = 'O' ;
         octets[10] = 'O' ;
         octets[11] = 'F' ;

     //    if (debug)
     //       dumpBytes(octets, origRtp.getLength()) ;

         send(origRtp) ;
      }

   /**
    * @param address Add this address to the list destinations for the forked RTP
    */
   public void addDestination(SocketAddress address)
   {
      if (address != null)
      {
         LOG.debug(String.format("RtpFork::addDestination(%s)", address)) ;
         destinations.add(address);
      }
   }

   /**
    * @param address Remove this address from the list of destinations for the forked RTP
    */
   public void removeDestination(SocketAddress address)
   {
      if (address != null)
      {
         LOG.debug(String.format("RtpFork::removeDestination(%s)", address)) ;
         destinations.remove(address) ;
      }
   }

   public void removeAllDestinations()
   {
      LOG.debug(String.format("RtpFork::removeAllDestinations")) ;
      destinations.clear() ;
   }

   public void stopLocalAudio()
   {
      LOG.debug(String.format("RtpFork::closeLocalAudio")) ;
      try
      {
         if (ulawStream != null)
         {
            ulawStream.close() ;
            ulawStream = null ;
         }
      } catch (Exception e) {}
      LegEvent event = new LegEvent(null, "localAudio end") ;
      legListener.onEvent(event) ;
   }

   public void startLocalAudio(URL audioSource)
   {
      final AudioFormat ULAW_FORMAT_1 =
         new AudioFormat( AudioFormat.Encoding.ULAW,
                          8000f, //sample rate
                          8, //bits per sample
                          1, //channels
                          1, //frame rate
                          8000f, // frame size
                          false); //isBigEndian

      ulawStream = null ;
      String reason = "Unable to open "+ audioSource.toString() ;
      try
      {
         AudioInputStream inStream = AudioSystem.getAudioInputStream(audioSource) ;
         if (inStream.getFormat() == ULAW_FORMAT_1)
         {
            ulawStream = inStream ;
         }
         else
         {
            reason = "Unable to convert "+inStream.toString()+" to uLaw.";
            // Convert steam to uLaw if possible
            ulawStream = AudioSystem.getAudioInputStream(
                ULAW_FORMAT_1, inStream);
            reason = "success" ;
         }
      } catch (Exception e) {}
      finally
      {
         if (!reason.equals("success"))
         {
            LOG.debug(String.format("RtpFork::setLocalAudio(%s) failed: %s", audioSource, reason)) ;
            stopLocalAudio() ;
         }
         else
         {
            LOG.debug(String.format("RtpFork::setLocalAudio(%s) succeded: %s", audioSource, reason)) ;
         }
      }
   }

   /**
    * Get the next RTP packet of local audio.
    * @return A datagram of an RTP packet containing the audio payload, or null if none is available.
    */
   DatagramPacket getLocalAudioRtp()
   {
      if (ulawStream == null)
      {
         // No stream is available, return null ;
         return null ;
      }

      byte [] octets = rtpOutPacket.getData() ;
      int n = 0 ;
      try
      {
         // Read bytes from the local audio stream into the payload section of the RTP packet
         n = ulawStream.read(octets, 12, payloadSize) ;
         if (n > 0)
         {
            if (n < payloadSize)
            {
               for(; n<payloadSize; n++)
               {
                  octets[n+12] = (byte)0xff ; // Pad rest of payload with with uLaw silence
               }
            }

            // Fill in static RTP header bytes
            octets[0] = (byte)0x80 ;      // V=2, CC=0
            octets[1] = (byte)0 ;         // M=0, PT=0 (uLaw)

            return rtpOutPacket ;
         }
         LOG.debug("RtpFork::getLocalAudioRtp EOF on ulawStream") ;

      } catch (IOException e) {
         LOG.error("RtpFork::getLocalAudioRtp", e) ;
      }

      // When an error occurs or EOF is reached, close the stream
      stopLocalAudio() ;
      return null ;
   }

   /**
    * Beat (as in musical beat)
    * Call this method at a fixed rate periodically based on {@link rhythm rhythm}, or use
    * {@link start start(rhythm)} to do that for you in a seperate thread
    */
   void beat()
   {
      // This method needs to handle two manufacturers of JAVA:
      // SUN - always makes extra calls to catch up if it's late for a timeout.
      // IBM - may not make extra calls to catch up if it's late for a timeout.
      DatagramPacket localPacket;

      // Get the next packet of local audio (if any)
      if (nextAudioPacket == null)
      {
         localPacket = getLocalAudioRtp() ;
      }
      else
      {
         localPacket = nextAudioPacket ;
      }
      if (localPacket != null)
      {
         // Check if this is the first packet of the local audio
         if (firstTime == 0)
         {
            firstTime = System.currentTimeMillis() ;
            firstStamp = timestamp ;
         }

         // Calculate the ideal timestamp for the outgoing packet.
         long idealStamp = firstStamp + ((System.currentTimeMillis() - firstTime) * BYTES_PER_MS) ;

         if (timestamp <= idealStamp)
         {
            // Send it
            rtpSend(localPacket) ;
            nextAudioPacket = null;

            // Catch up if we haven't sent enough packets yet.
            for(int i=0; ((i<4) && (timestamp <= idealStamp)); i++)
            {
               // Send it and get the next packet
               localPacket = getLocalAudioRtp() ;
               if (localPacket != null)
               {
                  rtpSend(localPacket) ;
               }
               else
               {
                  break ;
               }
            }
         }
         else
         {
            // We can't send this audio packet yet. Hold it until later.
            nextAudioPacket = localPacket;
         }
      }
      if (localPacket == null)
      {
         // Reset ready for the next local audio.
         firstTime = 0 ;
      }

      /*
       * Check if packets (up to 5) are available to be read. If so, read and
       * load them into the jitter buffer.
       */
      for(int i=0; i<5; i++)
      {
         DatagramPacket packet = socket.read() ;
         if (localPacket != null)
         {
            // Ignore network packets if we are sending local packets
            continue ;
         }
         if (packet == null || packet.getLength() == 0)
         {
            break ;
         }
         // Add the packet into the jitter buffer
         jitterBuff.add(packet) ;
      }

      if (localPacket != null)
      {
         // Don't send network packets if we are sending local packets
         return ;
      }

      /*
       * Pull network packets (possibly re-ordered) out of the jitter buffer
       * and send them.
       */
      for(int i=0; i<5; i++)
      {
         // Remove the next packet from the jitter buffer
         DatagramPacket packet = jitterBuff.removeDatagram() ;
         if (packet != null)
         {
            rtpSend(packet) ;
         }
         else
         {
            break ;
         }
      }
   }

   /**
    * A {@link java.thread.run run} method to call beat in a thread.
    */
   public void run()
   {
      beat() ;
   }

   public void start()
   {
      /*
       * Using a single thread, read and retransmit packets every rhythm mS.
       * By doing it this way, inbound packets accumulate in the OS's IP buffers.  If the far
       * end is sending at the same rhythm, there will either be 0, 1 or 2 packets available to read,
       * depending on jitter and clock skew.
       *
       * By not waking up on every inbound packet, and instead polling perodically, we save effort,
       * and are more efficient.  It also means it is much easier to stop this, as we won't have
       * a thread blocked in I/O that we somehow have to cancel.
       */
      LOG.debug(String.format("RtpFork::start(%d)", rhythm)) ;
      service = new ScheduledThreadPoolExecutor(1) ;
      service.scheduleAtFixedRate(this, 0, rhythm, TimeUnit.MILLISECONDS);
   }

   public void stop()
   {
      if (service != null)
      {
         service.shutdown() ;
      }
   }

}
