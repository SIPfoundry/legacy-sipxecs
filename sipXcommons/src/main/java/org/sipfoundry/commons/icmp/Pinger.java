/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.icmp;

import static org.savarese.rocksaw.net.RawSocket.PF_INET;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.InetAddress;
import java.net.UnknownHostException;

import org.savarese.rocksaw.net.RawSocket;
import org.savarese.vserv.tcpip.*;

public class Pinger {
    protected InetAddress targetHost;
    protected int pingTimeout;
    protected int retry;
    protected RawSocket socket;
    protected ICMPEchoPacket sendPacket;
    protected ICMPEchoPacket recvPacket;
    protected int offset;
    protected int length;
    protected int dataOffset;
    protected int requestType;
    protected int replyType;
    protected byte[] sendData;
    protected byte[] recvData;
    protected byte[] srcAddress;
    protected int sequence;
    protected int identifier;

    public Pinger(int id, String host, int timeout) {
        sequence = 0;
        identifier = id;
        try {
            targetHost = InetAddress.getByName(host);
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }
        pingTimeout = timeout;

        srcAddress = new byte[4];
        requestType = ICMPPacket.TYPE_ECHO_REQUEST;
        replyType = ICMPPacket.TYPE_ECHO_REPLY;

        sendPacket = new ICMPEchoPacket(1);
        recvPacket = new ICMPEchoPacket(1);
        sendData = new byte[84];
        recvData = new byte[84];

        sendPacket.setData(sendData);
        recvPacket.setData(recvData);
        sendPacket.setIPHeaderLength(5);
        recvPacket.setIPHeaderLength(5);
        sendPacket.setICMPDataByteLength(56);
        recvPacket.setICMPDataByteLength(56);

        sendPacket.setType(requestType);
        sendPacket.setCode(0);
        sendPacket.setIdentifier(identifier);

        offset = sendPacket.getIPHeaderByteLength();
        dataOffset = offset + sendPacket.getICMPHeaderByteLength();
        length = sendPacket.getICMPPacketByteLength();

        socket = new RawSocket();
    }

    public boolean ping() {
        sendPacket.setSequenceNumber(sequence++);

        OctetConverter.longToOctets(System.nanoTime(), sendData, dataOffset);

        sendPacket.computeICMPChecksum();

        try {
            socket.open(PF_INET, 1);    // ICMP Socket.
        } catch (IOException e) {
            return false;
        }

        try {
            socket.setSendTimeout(pingTimeout);
            socket.setReceiveTimeout(pingTimeout);
        } catch (java.net.SocketException e1) {
            socket.setUseSelectTimeout(true);
        try {
            socket.setSendTimeout(pingTimeout);
            socket.setReceiveTimeout(pingTimeout);
        } catch (java.net.SocketException e2) {
            return false;
        }
        }

        try {
            socket.write(targetHost, sendData, offset, length);
        } catch (IOException e) {
            return false;
        }

        long start = System.currentTimeMillis();
        long end = start;

        do {
            try {
                socket.read(recvData, srcAddress);
            } catch (InterruptedIOException e) {
                return false;
            } catch (IOException e) {
                return false;
            }

            end = System.currentTimeMillis();

        } while ((recvPacket.getType() != replyType || recvPacket.getIdentifier() != identifier) && end - start < pingTimeout);

        try {
            socket.close();
        } catch (IOException e) {
            return false;
        }

        if (recvPacket.getType() == replyType && recvPacket.getIdentifier() == identifier) {
            return true;
        } else {
            return false;
        }
    }

}
