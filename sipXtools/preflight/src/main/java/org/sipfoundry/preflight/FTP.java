/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.io.*;
import java.net.*;

import org.apache.commons.net.*;
import org.apache.commons.net.ftp.*;
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
public class FTP {
    private static final int bindPort = 0;
    private static final String ftpUser = "PlcmSpIp";
    private static final String ftpPassword = "PlcmSpIp";

    public ResultCode validate(int timeout, NetworkResources networkResources, JournalService journalService,
            InetAddress bindAddress) {
        ResultCode results = NONE;
        InetAddress ftpServerAddress = null;
        String testFile = new String("00D01EFFFFFE");
        String[] verificationStrings = { "LIP-68XX configuration information", "[VOIP]", "outbound_proxy_server", "[PROVISION]",
                "decrypt_key" };

        if (networkResources.configServer == null) {
            journalService.println("No FTP server provided, skipping test.\n");
            return CONFIG_SERVER_MISSING;
        }

        journalService.println("Starting FTP server test.");

        if (IPAddressUtil.isLiteralIPAddress(networkResources.configServer)) {
            try {
                ftpServerAddress = InetAddress.getByName(networkResources.configServer);
            } catch (UnknownHostException e) {
                // Should never get here.
                e.printStackTrace();
            }
            journalService.println("Using FTP server literal address: " + networkResources.configServer);
        } else {
            // Try to retrieve A RECORD for FTP server, checking each DNS server.
            SimpleResolver resolver = null;
            try {
                resolver = new SimpleResolver();
                resolver.setLocalAddress(bindAddress);
                resolver.setTimeout(timeout);
            } catch (UnknownHostException e) {
                e.printStackTrace();
            }

            for (InetAddress dnsServer : networkResources.domainNameServers) {
                journalService.println("Looking up FTP server address via DNS server: " + dnsServer.getCanonicalHostName());
                String targetMessage = new String("  FTP server address \"" + networkResources.configServer + "\"");
                resolver.setAddress(dnsServer);
                Lookup aLookup = null;
                try {
                    aLookup = new Lookup(networkResources.configServer, Type.A);
                } catch (TextParseException e) {
                    journalService.println("  is malformed.\n");
                    journalService.println(targetMessage);
                    return FTP_ADDRESS_MALFORMED;
                }
                aLookup.setResolver(resolver);
                Record[] aRecords = aLookup.run();
                switch (aLookup.getResult()) {
                case Lookup.SUCCESSFUL:
                    if (aRecords != null) {
                        InetAddress targetAddress = ((ARecord) aRecords[0]).getAddress();
                        targetMessage += " resolves to: " + targetAddress.getHostAddress();
                        journalService.println(targetMessage);
                        if (ftpServerAddress == null) {
                            ftpServerAddress = targetAddress;
                        } else {
                            // Check that multiple lookups result in same
                            // address.
                            if (!ftpServerAddress.equals(targetAddress)) {
                                journalService.println("  FTP server address does not match prior lookup.");
                                results = MULTIPLE_CONFIG_TARGETS;
                            }
                        }
                    } else {
                        targetMessage += " could not be resolved.";
                        journalService.println(targetMessage);
                        results = FTP_TARGET_UNRESOLVED;
                    }
                    break;
                case Lookup.UNRECOVERABLE:
                    targetMessage += " [Unrecoverable error]";
                    journalService.println(targetMessage);
                    results = FTP_TARGET_UNRESOLVED;
                    break;
                case Lookup.TRY_AGAIN:
                    targetMessage += " [Lookup timeout]";
                    journalService.println(targetMessage);
                    results = FTP_TARGET_UNRESOLVED;
                    break;
                case Lookup.HOST_NOT_FOUND:
                    targetMessage += " could not be resolved.";
                    journalService.println(targetMessage);
                    results = FTP_TARGET_UNRESOLVED;
                    break;
                case Lookup.TYPE_NOT_FOUND:
                    targetMessage += " could not be resolved.";
                    journalService.println(targetMessage);
                    results = FTP_TARGET_UNRESOLVED;
                    break;
                }
            }
        }

        if ((ftpServerAddress == null) || (results == MULTIPLE_CONFIG_TARGETS)) {
            journalService.println("Cannot recover from previous errors, aborting FTP test.\n");
            return results;
        }

        journalService.println("Beginning FTP get request of test file: " + testFile);

        // Open the FTP connection.
        FTPClient ftp = new FTPClient();
        ftp.setDefaultTimeout(timeout * 1000);
        ftp.addProtocolCommandListener(new PrintCommandListener(journalService));

        try {
            int reply;
            ftp.connect(ftpServerAddress, 21, bindAddress, bindPort);

            // After connection, check reply code to verify.
            reply = ftp.getReplyCode();
            if (!FTPReply.isPositiveCompletion(reply)) {
                ftp.disconnect();
                journalService.println("FTP client failure: " + reply + "\n");
                return FTP_CLIENT_FAILURE;
            }
        } catch (IOException e) {
            if (ftp.isConnected()) {
                try {
                    ftp.disconnect();
                } catch (IOException f) {
                    // Ignore.
                }
            }
            journalService.println("FTP client failure: " + e.getMessage() + "\n");
            return FTP_CLIENT_FAILURE;
        }

        try {
            if (!ftp.login(ftpUser, ftpPassword)) {
                ftp.logout();
                journalService.println("FTP client unable to log in.\n");
                return FTP_GET_FAILED;
            }

            journalService.println("FTP client connected to: " + ftp.getSystemName());

            ftp.enterLocalPassiveMode();

            ByteArrayOutputStream output = new ByteArrayOutputStream();
            ftp.retrieveFile(testFile, output);

            // After receiving, check reply code to verify.
            int reply = ftp.getReplyCode();
            if (!FTPReply.isPositiveCompletion(reply)) {
                ftp.disconnect();
                journalService.println("FTP get failure: " + reply + "\n");
                return FTP_GET_FAILED;
            }

            ftp.logout();

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
                System.err.println(testFileContents);
                results = FTP_CONTENTS_FAILED;
            }
        } catch (FTPConnectionClosedException e) {
            journalService.println("FTP server closed connection prematurely.\n");
            return FTP_GET_FAILED;
        } catch (IOException e) {
            journalService.println("FTP client failure. " + e.getMessage() + "\n");
            return FTP_CLIENT_FAILURE;
        } finally {
            if (ftp.isConnected()) {
                try {
                    ftp.disconnect();
                } catch (IOException f) {
                    // Ignore.
                }
            }
        }

        journalService.println("");
        return results;
    }

    public class PrintCommandListener implements ProtocolCommandListener {
        private final JournalService journalService;

        public PrintCommandListener(JournalService journalService) {
            this.journalService = journalService;
        }

        public void protocolCommandSent(ProtocolCommandEvent event) {
            journalService.print("  " + event.getMessage());
        }

        public void protocolReplyReceived(ProtocolCommandEvent event) {
            journalService.print("  " + event.getMessage());
        }
    }

}
