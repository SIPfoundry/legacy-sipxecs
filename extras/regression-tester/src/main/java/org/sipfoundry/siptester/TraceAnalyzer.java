package org.sipfoundry.siptester;

import gov.nist.javax.sip.message.SIPRequest;
import gov.nist.javax.sip.message.SIPResponse;

import java.util.Collection;
import java.util.Iterator;

import javax.sip.message.Request;

import org.apache.log4j.Logger;

public class TraceAnalyzer {

    private static Logger logger = Logger.getLogger(TraceAnalyzer.class);
    private String mergedFileName;

    public TraceAnalyzer(String mergedFileName) {
        this.mergedFileName = mergedFileName;
    }

    public void analyze() {
        Collection<CapturedLogPacket> capturedLogPackets = new LogFileParser()
                .parseXml(mergedFileName);
        Iterator<CapturedLogPacket> it = capturedLogPackets.iterator();
        logger.debug("found " + capturedLogPackets.size() + " packets ");

        while (it.hasNext()) {
            CapturedLogPacket capturedLogPacket = it.next();
            logger.debug("examining " + capturedLogPacket.getSipPacket());
            if (capturedLogPacket.getDestinationPort() == 0) {
                HostPort hostPort = CapturedLogPacket.hostMapper.get(capturedLogPacket
                        .getDestinationAddess());
                if (hostPort != null) {
                    capturedLogPacket.setDestinationAddress(hostPort.getIpAddress());
                    capturedLogPacket.setDestinationPort(hostPort.getPort());
                }
            }
            logger.debug("destAddress = " + capturedLogPacket.getDestinationAddess() + " src "
                    + capturedLogPacket.getSourceAddress());

            if (capturedLogPacket.getSipPacket() instanceof Request) {
                String address = capturedLogPacket.getDestinationAddess();
                SIPRequest sipRequest = ((SIPRequest) capturedLogPacket.getSipPacket());

                int port = capturedLogPacket.getDestinationPort();
               

                EmulatedEndpoint destEndpoint = SipTester.getEmulatedEndpoint(address, port);
                String transactionid = sipRequest.getTransactionId().toLowerCase();
                String viaAddress = capturedLogPacket.getTopmostViaHost();
                int viaPort = capturedLogPacket.getTopmostViaPort();
                if (viaPort == -1) {
                    viaPort = 5060;
                }
                EmulatedEndpoint sourceEndpoint = SipTester.getEmulatedEndpoint(viaAddress, viaPort);
                logger.debug("adding request " + sipRequest.getMethod() + " frameId "
                        + capturedLogPacket.getFrameId() + " source " + address + " port " + port );
                if (sourceEndpoint != destEndpoint) {
                    SipClientTransaction clientTx = SipTester.clientTransactionMap
                            .get(transactionid);

                    SipRequest srequest = null;
                    if (clientTx == null) {
                        srequest = new SipRequest(sipRequest, capturedLogPacket.getTimeStamp(),
                                capturedLogPacket.getFrameId());
                        String destinationAddress = capturedLogPacket.getDestinationAddess();
                        int destinationPort = capturedLogPacket.getDestinationPort();
                        srequest.setTargetHostPort(new HostPort(destinationAddress,destinationPort));
                        clientTx = SipTester.addTransmittedSipRequest(srequest);
                        if (sourceEndpoint != null) {
                            sourceEndpoint.addEmulatedClientTransaction(clientTx);
                        }
                    }

                    SipServerTransaction serverTx = SipTester.serverTransactionMap
                            .get(transactionid);

                    if (serverTx == null) {
                        if (srequest == null) {
                            srequest = new SipRequest(sipRequest, capturedLogPacket
                                    .getTimeStamp(), capturedLogPacket.getFrameId());
                        }
                        serverTx = SipTester.addReceivedSipRequest(srequest);
                        if (destEndpoint != null) {
                            destEndpoint.addEmulatedServerTransaction(serverTx);
                        }
                    }
                }

            } else {
                SipResponse sipResponse = new SipResponse((SIPResponse) capturedLogPacket
                        .getSipPacket(), capturedLogPacket.getTimeStamp(), capturedLogPacket
                        .getFrameId());

                String transactionId = capturedLogPacket.getTransactionId();
                logger.debug("checking sipResponse "
                        + capturedLogPacket.getSipPacket().getFirstLine() + " transactionId "
                        + transactionId);
                /*
                 * Find the server tx to which this response belongs.
                 */
                if (SipTester.getSipServerTransaction(transactionId) != null) {
                    logger.debug("found SipServerTransaction for message OUTGOING "
                            + transactionId);
                    /*
                     * Add the response to the server tx.
                     */
                    SipServerTransaction sst = SipTester.getSipServerTransaction(transactionId);
                    sst.addResponse(sipResponse);
                } else {
                    logger.debug("Could not find a Server Transaction for " + transactionId);
                }

                if (SipTester.getSipClientTransaction(transactionId) != null) {
                    logger.trace("found SipClientTransaction for message OUTGOING  "
                            + transactionId);
                    /*
                     * Find the endpoint that owns the client transaction corresponding to the
                     * response and add the packet.
                     */
                    SipClientTransaction sct = SipTester.getSipClientTransaction(transactionId);
                    sct.addResponse(sipResponse);
                } else {
                    logger.debug("Could not find a client tx corresponding to " + transactionId);
                }

            }
        }
    }

}
