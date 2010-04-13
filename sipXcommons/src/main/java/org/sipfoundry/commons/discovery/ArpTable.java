/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.discovery;

import java.io.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ArpTable {
    private static String procArpTableFile = "/proc/net/arp";

    public static String lookup(String ipAddress) {
        if (System.getProperty("os.name").toLowerCase().contains("windows")) {
            String macAddress =  windowsLookup(ipAddress);
            if ((macAddress == null)
             || (macAddress.compareTo("") == 0)
             || (macAddress.compareTo("00:00:00:00:00:00") == 0)) {
                return null;
            } else {
                return macAddress.toUpperCase();
            }
        } else {
            return linuxLookup(ipAddress);
        }
    }

    private static String linuxLookup(String ipAddress) {
        String macAddress = null;

        try {
            FileReader procArpTable = new FileReader(procArpTableFile);
            BufferedReader in = new BufferedReader(procArpTable);
            // Eat the column header descriptions.
            in.readLine();

            // Now search the remainder of the file, looking for a matching IP address.
            String line;
            while ((line = in.readLine()) != null) {
                String[] fields = line.split("\\s*\\s");
                if (fields.length > 3) {
                    if (ipAddress.compareTo(fields[0]) == 0) {
                        macAddress = fields[3].toUpperCase();
                        break;
                    }
                }
            }
            procArpTable.close();

        } catch (FileNotFoundException e) {
            return null;
        } catch (IOException e) {
            return null;
        }

        return macAddress;
    }

    private native static String windowsLookup(String ipAddress);

    public static void main(String args[]) {
        String macAddress = ArpTable.lookup("10.1.20.4");
        System.out.println(macAddress);
    }

}

class StreamBuffer extends Thread {
    InputStream inputStream;
    InputStreamReader inputStreamReader;
    BufferedReader bufferReader;
    String buffer = "";

    StreamBuffer(InputStream inputStream) {
            this.inputStream = inputStream;
    }

    String getBuffer() {
            return buffer;
    }

    public void run() {
            String tmpBuf;
            try {
                    inputStreamReader = new InputStreamReader(inputStream);
                    bufferReader = new BufferedReader(inputStreamReader);
                    while ((tmpBuf = bufferReader.readLine()) != null) {
                            buffer = buffer.concat(tmpBuf);
                    }
            } catch (IOException e) {
                    e.printStackTrace();
            }
    }
}
