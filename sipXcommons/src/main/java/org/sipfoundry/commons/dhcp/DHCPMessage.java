/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dhcp;

import java.io.ByteArrayInputStream;
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
public class DHCPMessage {
    private MessageType op;
    private HardwareAddressType htype;
    private int hlen; // Hardware address length.
    private int hops; // Optionally used by relay agents to record hops.
    private int xid; // Transaction ID.
    private int secs; // Seconds elapsed since client began request.
    private boolean broadcast; // broadcast value of "flags" field.
    private InetAddress ciaddr; // Client IP address.
    private InetAddress yiaddr; // 'your' (client) IP address.
    private InetAddress siaddr; // IP address of next server.
    private InetAddress giaddr; // Relay agent IP address.
    private byte[] chaddr; // Client hardware address.
    private String sname; // Optional server host name.
    private String file; // Boot file name.
    private LinkedList<DHCPOption> optionsList;

    public DHCPMessage() {
        chaddr = new byte[16];
        sname = "";
        file = "";
    }

    public MessageType getOp() {
        return op;
    }

    public void setOp(MessageType op) {
        this.op = op;
    }

    public HardwareAddressType getHtype() {
        return htype;
    }

    public void setHtype(HardwareAddressType htype) {
        this.htype = htype;
    }

    public int getHlen() {
        return hlen;
    }

    public void setHlen(int hlen) {
        this.hlen = hlen;
    }

    public int getHops() {
        return hops;
    }

    public void setHops(int hops) {
        this.hops = hops;
    }

    public int getXid() {
        return xid;
    }

    public void setXid(int xid) {
        this.xid = xid;
    }

    public int getSecs() {
        return secs;
    }

    public void setSecs(int secs) {
        this.secs = secs;
    }

    public boolean isBroadcast() {
        return broadcast;
    }

    public void setBroadcast(boolean broadcast) {
        this.broadcast = broadcast;
    }

    public InetAddress getCiaddr() {
        return ciaddr;
    }

    public void setCiaddr(InetAddress ciaddr) {
        this.ciaddr = ciaddr;
    }

    public InetAddress getYiaddr() {
        return yiaddr;
    }

    public void setYiaddr(InetAddress yiaddr) {
        this.yiaddr = yiaddr;
    }

    public InetAddress getSiaddr() {
        return siaddr;
    }

    public void setSiaddr(InetAddress siaddr) {
        this.siaddr = siaddr;
    }

    public InetAddress getGiaddr() {
        return giaddr;
    }

    public void setGiaddr(InetAddress giaddr) {
        this.giaddr = giaddr;
    }

    public byte[] getChaddr() {
        return chaddr;
    }

    public void setChaddr(String chaddrString) {
        StringTokenizer tokenizer = new StringTokenizer(chaddrString, ":");
        int chaddrLength = tokenizer.countTokens();
        for (int x = 0; x < chaddrLength; x++) {
            chaddr[x] = (byte) Integer.parseInt(tokenizer.nextToken(), 16);
        }
        // Pad with zero's.
        for (int x = chaddrLength; x < 16; x++) {
            chaddr[x] = 0;
        }

    }

    public String getSName() {
        return sname;
    }

    public void setSName(String sname) {
        this.sname = sname;
    }

    public String getFile() {
        return file;
    }

    public void setFile(String file) {
        this.file = file;
    }

    public LinkedList<DHCPOption> getOptions() {
        return optionsList;
    }

    public void addOption(DHCPOption option) {
        if (optionsList == null) {
            optionsList = new LinkedList<DHCPOption>();
        }
        optionsList.add(option);
    }

    public void setOptions(LinkedList<DHCPOption> options) {
        this.optionsList = options;
    }

    public byte[] marshal() {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        DataOutputStream dataStream = new DataOutputStream(byteStream);

        try {
            dataStream.writeByte(op.toInt());
            dataStream.writeByte(htype.toInt());
            dataStream.writeByte(hlen);
            dataStream.writeByte(hops);
            dataStream.writeInt(xid);
            dataStream.writeShort(secs);
            if (broadcast) {
                dataStream.writeShort(1);
            } else {
                dataStream.writeShort(0);
            }
            dataStream.write(ciaddr.getAddress(), 0, 4);
            dataStream.write(yiaddr.getAddress(), 0, 4);
            dataStream.write(siaddr.getAddress(), 0, 4);
            dataStream.write(giaddr.getAddress(), 0, 4);
            dataStream.write(chaddr, 0, 16);

            int snameLength = sname.length();
            dataStream.write(sname.getBytes("ISO-8859-1"), 0, snameLength);
            for (int x = snameLength; x < 64; x++)
                dataStream.writeByte(0);

            int fileLength = file.length();
            dataStream.write(file.getBytes("ISO-8859-1"), 0, fileLength);
            for (int x = fileLength; x < 128; x++)
                dataStream.writeByte(0);

            if (optionsList != null) {
                dataStream.writeInt(0x63825363); // Magic cookie.
                for (DHCPOption option : optionsList) {
                    byte[] tmp = option.marshal();
                    dataStream.write(tmp, 0, tmp.length);
                }
                dataStream.writeByte(255); // End of options.
            }
        } catch (IOException e) {
            System.err.println(e);
        }

        return byteStream.toByteArray();
    }

    public void unmarshal(byte[] buffer) {
        ByteArrayInputStream byteStream = new ByteArrayInputStream(buffer, 0, buffer.length);
        DataInputStream dataStream = new DataInputStream(byteStream);

        try {
            op = MessageType.toEnum(dataStream.readByte() & 0xFF);
            htype = HardwareAddressType.toEnum(dataStream.readByte() & 0xFF);
            hlen = dataStream.readByte() & 0xFF;
            hops = dataStream.readByte() & 0xFF;
            xid = dataStream.readInt();
            secs = dataStream.readShort();
            if (dataStream.readShort() == 0) {
                broadcast = false;
            } else {
                broadcast = true;
            }
            byte[] addressBuffer = new byte[4];
            byte[] stringBuffer = new byte[128];
            dataStream.readFully(addressBuffer, 0, 4);
            ciaddr = InetAddress.getByAddress(addressBuffer);
            dataStream.readFully(addressBuffer, 0, 4);
            yiaddr = InetAddress.getByAddress(addressBuffer);
            dataStream.readFully(addressBuffer, 0, 4);
            siaddr = InetAddress.getByAddress(addressBuffer);
            dataStream.readFully(addressBuffer, 0, 4);
            giaddr = InetAddress.getByAddress(addressBuffer);
            dataStream.readFully(chaddr, 0, 16);
            dataStream.readFully(stringBuffer, 0, 64);
            sname = new String(stringBuffer, 0, 64, "ISO-8859-1").trim();
            dataStream.readFully(stringBuffer, 0, 128);
            file = new String(stringBuffer, 0, 128, "ISO-8859-1").trim();

            // Check for options.
            if (dataStream.available() > 4) {
                // Check for valid cookie.
                if (dataStream.readInt() == 0x63825363) {
                    // Found valid cookie, read in the options.
                    optionsList = DHCPOption.unmarshalOptions(dataStream);
                }
            }
        } catch (IOException e) {
            System.err.println(e);
        }

    }
}
