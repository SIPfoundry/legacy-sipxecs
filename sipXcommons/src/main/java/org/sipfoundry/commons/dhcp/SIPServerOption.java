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
import java.util.StringTokenizer;

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
        super.setLength(super.getLength() + server.length() + 2);
    }

    public String toString() {
        String servers = "";
        if (serverList != null) {
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
        }
        return servers;
    }

    public byte[] marshal() throws IOException {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        if (serverList != null) {
            dataStream.writeByte(super.getCode().toInt());
            dataStream.writeByte(super.getLength());
            if (serverList.getFirst().hostName != null) {
                dataStream.writeByte(0); // Encoding type 0.

                // First parse all of the host names and build up a list of fields
                LinkedList<String> fields = new LinkedList<String>();
                for (SIPServer server : serverList) {
                    StringTokenizer tokens = new StringTokenizer(server.hostName, ".");
                    while (tokens.hasMoreTokens()) {
                        String token = tokens.nextToken();
                        fields.add(token);
                    }
                    fields.add("");
                }

                // Now encode the list.
                for (String field : fields) {
                    dataStream.write(field.length());
                    if (field.length() > 0) {
                        dataStream.write(field.getBytes("ISO-8859-1"));
                    }
                }
            } else if (serverList.getFirst().IPaddress != null) {
                dataStream.writeByte(1); // Encoding type 1.
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
        length -= 1;
        if (encoding == 0) {
            serverList = new LinkedList<SIPServer>();
            int fieldLength = 0;
            String field = null;
            String hostName = "";
            while (length > 0) {
                fieldLength = dataStream.readByte() & 0xFF;
                length -= 1;
                if (fieldLength > 0) {
                    if (hostName.length() > 0) {
                        hostName = hostName.concat(".");
                    }
                    byte[] stringBuffer = new byte[fieldLength];
                    dataStream.readFully(stringBuffer, 0, fieldLength);
                    field = new String(stringBuffer, 0, fieldLength, "ISO-8859-1").trim();
                    length -= fieldLength;
                    hostName = hostName.concat(field);
                } else {
                    SIPServer newServer = new SIPServer();
                    newServer.IPaddress = null;
                    newServer.hostName = hostName;
                    serverList.add(newServer);
                    hostName = "";
                }
            }
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
            // See if this is a proprietary form of the option used by SMC.
            byte[] stringBuffer = new byte[length + 1];
            stringBuffer[0] = (byte) (encoding & 0xFF);
            dataStream.readFully(stringBuffer, 1, length);
            String optionValue = new String(stringBuffer, 0, length + 1, "ISO-8859-1").trim().toLowerCase();
            if (optionValue.contains(":12000/cmcprov/login")) {
                // Extract host name from option value.
                int startIndex = 0;
                if (optionValue.startsWith("http")) {
                    startIndex = 7;
                }
                int endIndex = optionValue.indexOf(':', startIndex);
                SIPServer newServer = new SIPServer();
                newServer.IPaddress = null;
                newServer.hostName = optionValue.substring(startIndex, endIndex);
                serverList = new LinkedList<SIPServer>();
                serverList.add(newServer);
            }
        }
    }

}
