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
public class SIPServerOption extends DHCPOption {
    private LinkedList<SIPServer> serverList = null;

    public SIPServerOption() {
        super.setCode(Code.SIP_SERVER);
        super.setLength(1);
    }

    public LinkedList<SIPServer> getServerList() {
        return serverList;
    }

    public void addServer(InetAddress server) {
        if (serverList == null) {
            serverList = new LinkedList<SIPServer>();
        }
        SIPServer newServer = new SIPServer();
        newServer.IPaddress = server;
        newServer.hostName = null;
        serverList.add(newServer);
        super.setLength(super.getLength() + 4);
    }

    public void addServer(String server) {
        if (serverList == null) {
            serverList = new LinkedList<SIPServer>();
        }
        SIPServer newServer = new SIPServer();
        newServer.hostName = server;
        newServer.IPaddress = null;
        serverList.add(newServer);
        super.setLength(3);  // When option is using host name strings, this length value is bogus.
    }

    public String toString() {
        String servers = "";
        for (SIPServer server : serverList) {
            if (servers.length() > 0) {
                servers += ", ";
            }
            if (server.IPaddress != null) {
            	servers += server.IPaddress.getHostAddress();
            } else if (server.hostName != null) {
            	servers += server.hostName;
            }
        }
        return servers;
    }

    public byte[] marshal() throws IOException {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        if (serverList != null) {
            dataStream.writeByte(super.getCode().toInt());
            if (serverList.getFirst().hostName != null) {
            	;
            } else if (serverList.getFirst().IPaddress != null) {
            	dataStream.writeByte(super.getLength());
            	dataStream.writeByte(1);  // Encoding type 1.
            	for (SIPServer server : serverList) {
                	dataStream.write(server.IPaddress.getAddress(), 0, 4);
            	}
            } else {
            	throw new IOException();
            }
        }

        return byteStream.toByteArray();
    }

    public void unmarshal(DataInputStream dataStream) throws IOException {
        int length = dataStream.readByte() & 0xFF;
        if (length < 3) {
            throw new IOException();
        }
        super.setLength(length);
        int encoding = dataStream.readByte() & 0xFF;
        if (encoding == 0) {
        	;
        } else if (encoding == 1) {
            serverList = new LinkedList<SIPServer>();
        	byte[] addressBuffer = new byte[4];
        	while (length > 3) {
        		SIPServer newServer = new SIPServer();
            	dataStream.readFully(addressBuffer, 0, 4);
            	newServer.IPaddress = InetAddress.getByAddress(addressBuffer);
        		newServer.hostName = null;
        		serverList.add(newServer);
            	length -= 4;
        	}
        } else {
            throw new IOException();
        }
    }

}
