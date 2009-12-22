package org.sipfoundry.siptester;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.FileReader;
import java.util.regex.Pattern;

import javax.sip.message.Request;

import org.apache.log4j.Logger;

public class LogFileReader {

    private static Logger logger = Logger.getLogger(LogFileReader.class);
    private String logDirName;

    public LogFileReader(String logDirName) throws Exception {
        this.logDirName = logDirName;
        if (!new File(logDirName).exists()) {
            throw new SipTesterException("Cannot find log file " + logDirName);
        }

    }

    private long parseTime(String time) {
        String timeStr1 = time.split("T")[1];
        String timeStr = timeStr1.substring(0, timeStr1.length() - 1);
        String[] timeStrSplit = timeStr.split(":");
        long hours = Integer.parseInt(timeStrSplit[0]) * 60 * 60 * 1000;
        long minutes = Integer.parseInt(timeStrSplit[1]) * 60 * 1000;
        String[] secondsMicroseconds = timeStrSplit[2].split("\\.");
        long seconds = Integer.parseInt(secondsMicroseconds[0]) * 1000;
        long nanoseconds = Integer.parseInt(secondsMicroseconds[1]) / 1000;
        return hours + minutes + seconds + nanoseconds;

    }

    private String getAgent(String agentStr) {
        String[] split = agentStr.split(":");
        return split[6];
    }

    private String unescape(String message) {
        String message1 = message.replaceAll("\\\\r\\\\n", "\r\n");
        String message2 = message1.replaceAll("\\\\\"", "\"");
        String message3 = message2.replaceAll("\\n", "\n");
        return message3;
    }

    public void readTraces() throws Exception {
        File directory = new File(this.logDirName);
        File[] files = directory.listFiles(new FileFilter() {
            @Override
            public boolean accept(File pathname) {
                if (pathname.getName().endsWith("log") && pathname.getName().startsWith("sip")) {
                    return true;
                } else
                    return false;
            }
        });

        for (File file : files) {
            readTraceFromFile(file);
        }
    }

    public void readTraceFromFile(File logFile) throws Exception {
        logger.debug("readTraceFromFile : " + logFile.getName());
        FileReader fileReader = new FileReader(logFile);
        BufferedReader bufferedReader = new BufferedReader(fileReader);
        String line;
        String pattern = "\"";
        long prev = 0;
        while ((line = bufferedReader.readLine()) != null) {

            if (line.indexOf("INCOMING") != -1) {
                logger.trace("Processing INCOMING request");
                String[] split1 = line.split(pattern);
                String time = split1[1];
                long miliseconds = this.parseTime(time);
                String agent = getAgent(split1[2]);
                prev = miliseconds;
                int index = line.indexOf("message:\\n");
                String message = line.substring(index);

                String[] sipAddressMessage = message.split("----");
                String address = sipAddressMessage[1].split(":")[1];
                int port = Integer.parseInt(sipAddressMessage[2].split(":")[1].trim());
                String sipMessage = sipAddressMessage[3].substring(2, sipAddressMessage[3]
                        .indexOf("====================END===================="));
                boolean isSender = false;
                CapturedLogPacket capturedLogPacket = new CapturedLogPacket(
                        unescape(sipMessage), miliseconds,  sipMessage, port, isSender);

                if (capturedLogPacket.getSipPacket() instanceof Request) {
                    String viaHost = capturedLogPacket.getTopmostViaHost();
                    int viaPort = capturedLogPacket.getTopmostViaPort();
                    if (viaPort == -1) {
                        viaPort = 5060;
                    }
                    Endpoint srcEndpoint = SipTester.getEndpoint(viaHost, viaPort);
                    if (srcEndpoint != null) {
                        srcEndpoint.addOriginatingPacket(capturedLogPacket);
                    }
                } else {

                    String transactionId = capturedLogPacket.getTransactionId();
                    for (Endpoint endpoint : SipTester.getEndpoints()) {
                        /*
                         * Find the server tx to which this response belongs.
                         */
                        if (endpoint.getSipServerTransaction(transactionId) != null) {
                            /*
                             * Add the response to the server tx.
                             */
                            endpoint.addOriginatingPacket(capturedLogPacket);
                        }

                        if (endpoint.getSipClientTransaction(transactionId) != null) {
                            logger.trace("found SipClientTransaction for message INCOMING "
                                    + transactionId);
                            /*
                             * Find the endpoint that owns the client transaction corresponding to
                             * the response and add the packet.
                             */
                            endpoint.addReceivedPacket(capturedLogPacket);
                        }
                    }

                }
            } else if (line.indexOf("OUTGOING") != -1) {
                logger.trace("Processing OUTGOING message");
                String[] split1 = line.split(pattern);
                String time = split1[1];
                long miliseconds = this.parseTime(time);
                prev = miliseconds;
                int index = line.indexOf("message:\\n");
                if ( index == -1 ) {
                    index = line.indexOf("Message :\\n");
                }
                if ( index == -1 ) {
                    throw new SipTesterException("Bad file format");
                }
                String message = line.substring(index);
                String[] sipAddressMessage = message.split("----");
                String address = sipAddressMessage[1].split(":")[1];
                int port = Integer.parseInt(sipAddressMessage[2].split(":")[1].trim());
                String sipMessage = sipAddressMessage[3].substring(2);
                boolean isSender = true;
                CapturedLogPacket capturedLogPacket = new CapturedLogPacket(
                        unescape(sipMessage), miliseconds,  sipMessage, port, isSender);
               

                if (capturedLogPacket.getSipPacket() instanceof Request) {
                    Endpoint destEndpoint = SipTester.getEndpoint(address, port);
                    if (destEndpoint != null) {
                        destEndpoint.addReceivedPacket(capturedLogPacket);
                    }
                } else {
                    String transactionId = capturedLogPacket.getTransactionId();
                    for (Endpoint endpoint : SipTester.getEndpoints()) {
                        /*
                         * Find the server tx to which this response belongs.
                         */
                        if (endpoint.getSipServerTransaction(transactionId) != null) {
                            logger.trace("found SipServerTransaction for message OUTGOING "
                                    + transactionId);
                            /*
                             * Add the response to the server tx.
                             */
                            endpoint.addOriginatingPacket(capturedLogPacket);
                        }

                        if (endpoint.getSipClientTransaction(transactionId) != null) {
                            logger.trace("found SipClientTransaction for message OUTGOING  "
                                    + transactionId);
                            /*
                             * Find the endpoint that owns the client transaction corresponding to
                             * the response and add the packet.
                             */
                            endpoint.addReceivedPacket(capturedLogPacket);
                        }
                    }
                }

            }
        }

    }
}
