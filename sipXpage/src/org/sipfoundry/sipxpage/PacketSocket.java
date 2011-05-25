/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketException;


/**
 * An inbound UDP socket, bound to a particular port for
 * recieving RTP packets.
 * <p>
 * Wraps a DatagramSocket with a buffer and some helpers
 * that deal with the exceptions.
 *
 * @author Woof!
 *
 */
public class PacketSocket
{
   DatagramSocket socket ;
   byte[] buf = new byte[65536] ;


   /*
    * An inbound socket.  Setup a listen port, and a packet to hold the received datagram
    */
   public PacketSocket(int port) throws SocketException
   {
      socket = new DatagramSocket(port) ;
      socket.setSoTimeout(1) ; // TODO Need a 0 length timeout, or poll (ferping nio...)
   }

   public PacketSocket(DatagramSocket socket)
   {
      this.socket = socket ;
   }

   public DatagramPacket read()
   {
      DatagramPacket packet = new DatagramPacket(buf, buf.length);
      try
      {
         socket.receive(packet) ;
         return packet ;
      } catch (IOException e)
      {
         return null ;
      }
   }

   public void send(SocketAddress address, DatagramPacket packet)
   {
      try
      {
         packet.setSocketAddress(address) ;
         socket.send(packet);
      } catch (IOException e) {}
   }

   public InetSocketAddress getInetSocketAddress()
   {
      return new InetSocketAddress(socket.getLocalAddress(), socket.getLocalPort()) ;
   }
}
