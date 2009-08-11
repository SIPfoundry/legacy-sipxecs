/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dhcp;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.util.LinkedList;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ServerOption extends DHCPOption {
    private LinkedList<InetAddress> serverList;

    public ServerOption(Code code) {
        super.setCode(code);
    }

    public LinkedList<InetAddress> getServerList() {
        return serverList;
    }

    public void addServer(InetAddress server) {
        if (serverList == null) {
            serverList = new LinkedList<InetAddress>();
        }
        serverList.add(server);
        super.setLength(super.getLength() + 4);
    }

    public String toString() {
        String servers = "";
        for (InetAddress server : serverList) {
            if (servers.length() > 0) {
                servers += ", ";
            }
            servers += server.getHostAddress();
        }
        return servers;
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        if (serverList != null) {
            try {
                dataStream.writeByte(super.getCode().toInt());
                dataStream.writeByte(super.getLength());
                for (InetAddress server : serverList) {
                    dataStream.write(server.getAddress(), 0, 4);
                }
            } catch (IOException e) {
                System.err.println(e);
            }
        }

        return byteStream.toByteArray();
    }

    public void unmarshal(DataInputStream dataStream) throws IOException {
        int length = dataStream.readByte() & 0xFF;
        if (length < 4) {
            throw new IOException();
        }
        super.setLength(length);
        serverList = new LinkedList<InetAddress>();
        byte[] addressBuffer = new byte[4];
        InetAddress server;
        while (length > 3) {
            dataStream.readFully(addressBuffer, 0, 4);
            server = InetAddress.getByAddress(addressBuffer);
            serverList.add(server);
            length -= 4;
        }
    }

}
