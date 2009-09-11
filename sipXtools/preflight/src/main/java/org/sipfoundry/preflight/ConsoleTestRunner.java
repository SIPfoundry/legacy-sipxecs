/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.net.*;
import java.util.LinkedList;

import org.apache.commons.cli.*;

import org.sipfoundry.commons.discovery.*;
import org.sipfoundry.commons.dhcp.NetworkResources;
import org.sipfoundry.commons.util.JournalService;

import static org.sipfoundry.preflight.ResultCode.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ConsoleTestRunner {
    private final JournalService journalService;

    ConsoleTestRunner(JournalService journalService) {
        this.journalService = journalService;
    }

    @SuppressWarnings("static-access")
    public void validate(String[] args) {
        InetAddress bindAddress = null;
        ResultCode results;
        NetworkResources networkResources = new NetworkResources();

        // create the command line parser
        CommandLineParser parser = new PosixParser();

        // create the Options
        Options options = new Options();
        Option verbose = OptionBuilder.withLongOpt("verbose").withDescription("Enable verbose test progress output.").create('v');
        Option reportProgress = OptionBuilder.withLongOpt("report-progress").withDescription("Enable discovery progress output.")
                .create();

        Option dhcpTest = OptionBuilder.withLongOpt("dhcp-test").withDescription(
                "Verify that the networks DHCP server is running and properly issuing IP addresses.").create();

        Option dnsTest = OptionBuilder.withLongOpt("dns-test").withDescription(
                "Verify that the DNS server(s) supplied by the DHCP server can properly resolve the given sip domain.")
                .withValueSeparator('=').hasArg().withArgName("realm").create();

        Option ntpTest = OptionBuilder.withLongOpt("ntp-test").withDescription(
                "Verify that the NTP server(s) supplied by the DHCP server are properly servicing NTP time requests.").withValueSeparator('=').hasOptionalArg().withArgName(
                "server").create();

        Option tftpTest = OptionBuilder.withLongOpt("tftp-test").withDescription(
                "Verify that the specified TFTP server is functioning properly.").withValueSeparator('=').hasOptionalArg().withArgName("server").create();

        Option ftpTest = OptionBuilder.withLongOpt("ftp-test").withDescription(
                "Verify that the specified FTP server is functioning properly.").withValueSeparator('=').hasOptionalArg().withArgName("server").create();

        Option httpTest = OptionBuilder.withLongOpt("http-test").withDescription(
                "Verify that the specified HTTP server is functioning properly.").withValueSeparator('=').hasOptionalArg().withArgName("server").create();

        Option sipTest = OptionBuilder.withLongOpt("120-test").withDescription(
                "Verify that DHCP server is properly issuing Option 120 addresses.").create();

        Option discover = OptionBuilder.withLongOpt("discover").withDescription(
                "Attempt to discover SIP devices on the attached subnet.").create();

        Option testInterface = OptionBuilder.withLongOpt("interface").withDescription(
                "IP address of the interface that the tests should run over.").withValueSeparator('=').hasArg().withArgName("address").create();

        Option help = OptionBuilder.withLongOpt("help").withDescription("Display preflight usage documentation.").create();

        options.addOption(dhcpTest);
        options.addOption(dnsTest);
        options.addOption(ntpTest);
        options.addOption(tftpTest);
        options.addOption(ftpTest);
        options.addOption(httpTest);
        options.addOption(sipTest);
        options.addOption(verbose);
        options.addOption(reportProgress);
        options.addOption(discover);
        options.addOption(testInterface);
        options.addOption(help);

        // Check that there is at least 1 argument.
        if (args.length < 1) {
            printHelp(options);
            System.exit(0);
        }

        try {
            // parse the command line arguments
            CommandLine line = parser.parse(options, args);

            if (line.hasOption("help")) {
                String helpText = "usage: preflight [-v, --verbose] [--dhcp-test] [--dns-test=<realm>] [--ntp-test]\n"
                        + "                 [--tftp-test=<server>] [--ftp-test=<server] [--http-test <server>]" + "\n"
                        + "Invoke preflight with one or more of the following test switches.  Each test will\n"
                        + "be run in the order specified.  If a given test fails, the program will terminate,\n"
                        + "returning an exit code specifying the error condition.  The supported test switches\n" + "are:\n" + "\n"
                        + "-v, --verbose        Enable verbose test progress output.\n" + "\n"
                        + "--dhcp-test          Verify that the networks DHCP server is running and properly\n"
                        + "                     issuing IP addresses.  Possible error conditions:\n"
                        + "                       130: Timeout waiting for network response.\n"
                        + "                       133: Multiple DHCP servers detected.\n"
                        + "                       134: Unrecognized response received.\n"
                        + "                       135: DHCP DISCOVER was rejected.\n"
                        + "                       136: DHCP REQUEST was rejected.\n"
                        + "                       137: Missing mandatory DHCP configuration parameters.\n" + "\n"
                        + "--dns-test=<sip domain> Verify that the DNS server(s) supplied by the DHCP server can\n"
                        + "                     properly resolve the given sip domain.  Possible error conditions:\n"
                        + "                       139: SRV target could not be resolved.\n"
                        + "                       140: SRV target is unreachable.\n"
                        + "                       141: Unable to resolve SIP domain.\n"
                        + "                       142: SIP domain is unreachable.\n"
                        + "                       143: DNS Server did not report any SRV records.\n" + "\n"
                        + "--ntp-test           Verify that the NTP server(s) supplied by the DHCP server are\n"
                        + "                     properly servicing NTP time requests.  Possible error conditions:\n"
                        + "                       144: No NTP servers available.\n"
                        + "                       146: NTP Server request failure.\n" + "\n"
                        + "--tftp-test=<server> Verify that the specified TFTP server is functioning properly.\n"
                        + "                     Possible error conditions:\n"
                        + "                       147: No TFTP server available.\n"
                        + "                       148: TFTP Server address is malformed.\n"
                        + "                       149: TFTP server address could not be resolved.\n"
                        + "                       150: Mismatch in DNS configuration server records.\n"
                        + "                       151: TFTP server is unreachable.\n"
                        + "                       152: TFTP client encountered unrecoverable error.\n"
                        + "                       153: TFTP get of test file failed.\n"
                        + "                       154: TFTP test file did not verify.\n" + "\n"
                        + "--ftp-test=<server>  Verify that the specified FTP server is functioning properly.\n"
                        + "                     Possible error conditions:\n"
                        + "                       155: FTP Server address is malformed.\n"
                        + "                       156: FTP server address could not be resolved.\n"
                        + "                       157: FTP server is unreachable.\n"
                        + "                       158: FTP client encountered unrecoverable error.\n"
                        + "                       159: FTP get of test file failed.\n"
                        + "                       160: FTP test file did not verify.\n" + "\n"
                        + "--http-test=<server> Verify that the specified HTTP server is functioning properly.\n"
                        + "                     Possible error conditions:\n"
                        + "                       161: HTTP URL is malformed.\n"
                        + "                       162: HTTP server address could not be resolved.\n"
                        + "                       163: HTTP server is unreachable.\n"
                        + "                       164: HTTP client encountered unrecoverable error.\n"
                        + "                       165: HTTP get of test file failed.\n"
                        + "                       166: HTTP test file did not verify.\n" + "\n"
                        + "--120-test           Verify that the DHCP server is properly issuing\n"
                        + "                     Option 120 addresses.  Possible error conditions:\n"
                        + "                       167: No SIP servers supplied.\n"
                        + "                       168: No SIP server is reachable.\n"
                        + "--interface=<address> IP address of the interface that the tests should run over.\n" + "\n"
                        + "--discover           Attempt to discover SIP devices on the attached subnet.\n" + "\n";
                System.out.println(helpText);
                System.exit(0);
            }

            if (line.hasOption("verbose")) {
                journalService.enable();
            } else {
                journalService.disable();
            }

            String interfaceAddress;
            if (line.hasOption("interface")) {
                interfaceAddress = line.getOptionValue("interface");
            } else {
                interfaceAddress = "0.0.0.0";
            }
            try {
                bindAddress = InetAddress.getByName(interfaceAddress);
            } catch (UnknownHostException e) {
                System.err.println(e.getMessage());
                System.exit(-1);
            }

            if (line.hasOption("discover")) {
                DHCP dhcp = new DHCP();
                String networkAddress = null;
                String networkMask = null;
                InetAddress hostAddress = null;
                InetAddress inetAddress;
                String localHostAddress = "0.0.0.0";

                results = dhcp.validate(10, networkResources, journalService, bindAddress);
                if (results != NONE) {
                    // Unable to determine the network mask via DHCP.
                    try {
                        // Default to a Class C subnet.
                        journalService.println("\nUnable to determine network mask via DHCP, defaulting to Class C.\n");
                        networkResources.subnetMask = InetAddress.getByName("255.255.255.0");
                    } catch (UnknownHostException e1) {
                        // Ignore.
                    }
                }

                try {
                    // Get local IP Address
                    inetAddress = InetAddress.getLocalHost();
                    localHostAddress = inetAddress.getHostAddress();
                } catch (UnknownHostException e) {
                    System.err.println(LOCAL_HOST_FAILURE.toString());
                    System.exit(LOCAL_HOST_FAILURE.toInt());
                }

                DiscoveryService ds = new DiscoveryService(localHostAddress, 5050);

                try {
                    // Get local IP Address
                    hostAddress = InetAddress.getLocalHost();
                    localHostAddress = hostAddress.getHostAddress();
                } catch (UnknownHostException e1) {
                    // Ignore.
                }

                // Calculate the network address.
                byte[] rawMaskAddress;
                byte[] rawHostAddress;
                byte[] rawNetworkAddress = new byte[4];
                rawMaskAddress = networkResources.subnetMask.getAddress();
                rawHostAddress = hostAddress.getAddress();
                for (int i = 3; i >= 0; i--) {
                    rawNetworkAddress[i] = (byte) (rawHostAddress[i] & rawMaskAddress[i]);
                }
                try {
                    networkAddress = InetAddress.getByAddress(rawNetworkAddress).getHostAddress();
                    networkMask = InetAddress.getByAddress(rawMaskAddress).getHostAddress();
                } catch (UnknownHostException e) {
                    // Ignore.
                }

                LinkedList<Device> devices = ds.discover(networkAddress, networkMask, line.hasOption("report-progress"));

                journalService.enable();

                if (devices == null) {
                    journalService.println("No devices found.");
                } else {
                    // Dump out the list of discovered devices.
                    journalService.println("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
                    journalService.println("<devices xmlns='http://www.sipfoundry.org/sipX/schema/xml/devices-00-00'>");
                    for (Device device : devices) {
                        journalService.println("    <device>");
                        journalService.print("        <hardware-address>");
                        journalService.print(device.getHardwareAddress());
                        journalService.println("</hardware-address>");
                        journalService.print("        <network-address>");
                        journalService.print(device.getNetworkAddress().getHostAddress());
                        journalService.println("</network-address>");
                        journalService.print("        <vendor>");
                        journalService.print(device.getVendor());
                        journalService.println("</vendor>");
                        journalService.print("        <user-agent>");
                        journalService.print(device.getUserAgentInfo());
                        journalService.println("</user-agent>");
                        journalService.println("    </device>");
                    }
                    journalService.println("</devices>");
                }

                System.exit(0);
            }

            // Always run the DHCP test first, regardless of what other tests are called for.
            DHCP dhcp = new DHCP();
            results = dhcp.validate(10, networkResources, journalService, bindAddress);
            if (results != NONE) {
                System.err.println(results.toString());
                System.exit(results.toInt());
            }

            if (line.hasOption("dns-test")) {
                networkResources.sipDomainName = line.getOptionValue("dns-test");
                DNS dns = new DNS();
                results = dns.validate(10, networkResources, journalService, bindAddress);
                if (results != NONE) {
                    System.err.println(results.toString());
                    System.exit(results.toInt());
                }
            }

            if (line.hasOption("ntp-test")) {
                NTP ntp = new NTP();
                String ntpServer = line.getOptionValue("ntp-test");
                if (ntpServer != null) {
                    networkResources.ntpServers = new LinkedList<InetAddress>();
                    try {
                        networkResources.ntpServers.add(InetAddress.getByName(ntpServer));
                    } catch (UnknownHostException e) {
                        journalService.println("Invalid NTP server specified on command line.");
                        networkResources.ntpServers = null;
                    }
                }
                results = ntp.validate(10, networkResources, journalService, bindAddress);
                if (results != NONE) {
                    System.err.println(results.toString());
                    System.exit(results.toInt());
                }
            }

            if (line.hasOption("tftp-test")) {
                TFTP tftp = new TFTP();
                String configServer = line.getOptionValue("tftp-test");
                if (configServer != null) {
                	networkResources.configServer = configServer;
                }
                results = tftp.validate(10, networkResources, journalService, bindAddress);
                if (results != NONE) {
                    System.err.println(results.toString());
                    System.exit(results.toInt());
                }
            }

            if (line.hasOption("ftp-test")) {
                FTP ftp = new FTP();
                String configServer = line.getOptionValue("ftp-test");
                if (configServer != null) {
                	networkResources.configServer = configServer;
                }
                results = ftp.validate(10, networkResources, journalService, bindAddress);
                if (results != NONE) {
                    System.err.println(results.toString());
                    System.exit(results.toInt());
                }
            }

            if (line.hasOption("http-test")) {
                HTTP http = new HTTP();
                String configServer = line.getOptionValue("http-test");
                if (configServer != null) {
                	networkResources.configServer = configServer;
                }
                results = http.validate(10, networkResources, journalService, bindAddress);
                if (results != NONE) {
                    System.err.println(results.toString());
                    System.exit(results.toInt());
                }
            }

            if (line.hasOption("120-test")) {
                SIPServerTest sipServerTest = new SIPServerTest();
                results = sipServerTest.validate(10, networkResources, journalService, bindAddress);
                if (results != NONE) {
                    System.err.println(results.toString());
                    System.exit(results.toInt());
                }
            }

        } catch (ParseException exp) {
            System.out.println(exp.getMessage());
            printHelp(options);
            System.exit(0);
        }

    }

    private void printHelp(Options options) {
        HelpFormatter formatter = new HelpFormatter();
        String header = "Invoke preflight with one or more of the following test switches.  Each "
                + "test will be run in the order specified.  If a given test fails, the "
                + "program will terminate, returning an exit code specifying the error "
                + "condition.  The supported test switches are:";
        formatter.printHelp("preflight", header, options, "", true);
    }

}
