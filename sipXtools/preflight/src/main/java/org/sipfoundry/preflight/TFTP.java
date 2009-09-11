/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.io.*;
import java.net.*;

import org.apache.commons.net.tftp.*;
import org.xbill.DNS.*;

import org.sipfoundry.commons.dhcp.NetworkResources;
import org.sipfoundry.commons.util.JournalService;
import org.sipfoundry.commons.util.IPAddressUtil;

import static org.sipfoundry.preflight.ResultCode.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class TFTP {
    private static final int bindPort = 0;

    public ResultCode validate(int timeout, NetworkResources networkResources, JournalService journalService,
            InetAddress bindAddress) {
        ResultCode results = NONE;
        InetAddress tftpServerAddress = null;
        String testFile = new String("00D01EFFFFFE");
        String[] verificationStrings = { "LIP-68XX configuration information", "[VOIP]", "outbound_proxy_server", "[PROVISION]",
                "decrypt_key" };

        if (networkResources.configServer == null) {
            journalService.println("No TFTP server provided, skipping test.\n");
            return CONFIG_SERVER_MISSING;
        }

        journalService.println("Starting TFTP server test.");

        TFTPClient tftp = new TFTPClient();

        tftp.setDefaultTimeout(timeout * 1000);

        if (IPAddressUtil.isLiteralIPAddress(networkResources.configServer)) {
            try {
                tftpServerAddress = InetAddress.getByName(networkResources.configServer);
            } catch (UnknownHostException e) {
                // Should never get here.
                e.printStackTrace();
            }
            journalService.println("Using TFTP server literal address: " + networkResources.configServer);
        } else {
            // Try to retrieve A RECORD for TFTP server, checking each DNS server.
            SimpleResolver resolver = null;
            try {
                resolver = new SimpleResolver();
                resolver.setTimeout(timeout);
            } catch (UnknownHostException e) {
                e.printStackTrace();
            }

            for (InetAddress dnsServer : networkResources.domainNameServers) {
                journalService.println("Looking up TFTP server address via DNS server: " + dnsServer.getCanonicalHostName());
                String targetMessage = new String("  TFTP server address \"" + networkResources.configServer + "\"");
                resolver.setAddress(dnsServer);
                Lookup aLookup = null;
                try {
                    aLookup = new Lookup(networkResources.configServer, Type.A);
                } catch (TextParseException e) {
                    journalService.println("  is malformed.\n");
                    journalService.println(targetMessage);
                    return TFTP_ADDRESS_MALFORMED;
                }
                aLookup.setResolver(resolver);
                Record[] aRecords = aLookup.run();
                switch (aLookup.getResult()) {
                case Lookup.SUCCESSFUL:
                    if (aRecords != null) {
                        InetAddress targetAddress = ((ARecord) aRecords[0]).getAddress();
                        targetMessage += " resolves to: " + targetAddress.getHostAddress();
                        journalService.println(targetMessage);
                        if (tftpServerAddress == null) {
                            tftpServerAddress = targetAddress;
                        } else {
                            // Check that multiple lookups result in same address.
                            if (!tftpServerAddress.equals(targetAddress)) {
                                journalService.println("  TFTP server address does not match prior lookup.");
                                results = MULTIPLE_CONFIG_TARGETS;
                            }
                        }
                    } else {
                        targetMessage += " could not be resolved.";
                        journalService.println(targetMessage);
                        results = TFTP_TARGET_UNRESOLVED;
                    }
                    break;
                case Lookup.UNRECOVERABLE:
                    targetMessage += " [Unrecoverable error]";
                    journalService.println(targetMessage);
                    results = TFTP_TARGET_UNRESOLVED;
                    break;
                case Lookup.TRY_AGAIN:
                    targetMessage += " [Lookup timeout]";
                    journalService.println(targetMessage);
                    results = TFTP_TARGET_UNRESOLVED;
                    break;
                case Lookup.HOST_NOT_FOUND:
                    targetMessage += " could not be resolved.";
                    journalService.println(targetMessage);
                    results = TFTP_TARGET_UNRESOLVED;
                    break;
                case Lookup.TYPE_NOT_FOUND:
                    targetMessage += " could not be resolved.";
                    journalService.println(targetMessage);
                    results = TFTP_TARGET_UNRESOLVED;
                    break;
                }
            }
        }

        if ((tftpServerAddress == null) || (results == MULTIPLE_CONFIG_TARGETS)) {
            journalService.println("Cannot recover from previous errors, aborting TFTP test.\n");
            return results;
        }

        journalService.println("Beginning TFTP get request of test file: " + testFile);
        // Open local socket
        try {
            tftp.open();
        } catch (SocketException e) {
            journalService.println("TFTP client failure. " + e.getMessage() + "\n");
            return TFTP_CLIENT_FAILURE;
        }

        // Try to receive remote file via TFTP
        try {
            ByteArrayOutputStream output = new ByteArrayOutputStream();
            tftp.open(bindPort, bindAddress);
            tftp.receiveFile(testFile, org.apache.commons.net.tftp.TFTP.ASCII_MODE, output, tftpServerAddress);
            String testFileContents = output.toString();
            boolean verified = true;
            for (String verificationString : verificationStrings) {
                if (!testFileContents.contains(verificationString)) {
                    verified = false;
                }
            }
            if (verified) {
                journalService.println("File received successfully.");
            } else {
                journalService.println("File received but contents do not verify.");
                results = TFTP_CONTENTS_FAILED;
            }
        } catch (UnknownHostException e) {
            journalService.println("TFTP client failure. " + e.getMessage());
            results = TFTP_CLIENT_FAILURE;
        } catch (IOException e) {
            journalService.println("TFTP get failed. " + e.getMessage());
            results = TFTP_GET_FAILED;
        } finally {
            // Close local socket and output file
            tftp.close();
        }

        journalService.println("");
        return results;
    }

}
