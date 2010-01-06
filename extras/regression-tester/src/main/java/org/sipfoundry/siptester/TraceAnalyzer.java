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
        Collection<CapturedLogPacket> capturedLogPackets = new LogFileParser().parseXml( mergedFileName);
        Iterator<CapturedLogPacket> it = capturedLogPackets.iterator();
        logger.debug("found " + capturedLogPackets.size() + " packets ");
        
        while (it.hasNext()) {
            CapturedLogPacket capturedLogPacket = it.next();
            logger.debug("examining " + capturedLogPacket.getSipPacket());
            if (capturedLogPacket.getDestinationPort() == 0) {
                HostPort hostPort = CapturedLogPacket.hostMapper.get(capturedLogPacket
                        .getDestinationAddess());
                if (hostPort != null) {
                    capturedLogPacket.setDestinationAddress(hostPort.getHost());
                    capturedLogPacket.setDestinationPort(hostPort.getPort());
                }
            }
            logger.debug("destAddress = " + capturedLogPacket.getDestinationAddess() + " src "
                    + capturedLogPacket.getSourceAddress());

            if (capturedLogPacket.getSipPacket() instanceof Request) {
                String address = capturedLogPacket.getDestinationAddess();
                SIPRequest sipRequest = ((SIPRequest)capturedLogPacket.getSipPacket());
                
                int port = capturedLogPacket.getDestinationPort();
                logger.debug("adding request " + sipRequest.getMethod() + " transactionId " + sipRequest.getTransactionId()  );
                
                EmulatedEndpoint destEndpoint = SipTester.getEndpoint(address, port);
                if (destEndpoint != null) {
                    destEndpoint.addReceivedPacket(capturedLogPacket);
                    logger.debug(" destEndpoint " + destEndpoint.getIpAddress() + ":" + destEndpoint.getPort());
                } else {
                    logger.debug(" destEndpoint " + address + ":" + port + " is not emulated ");
                }
                String viaAddress = capturedLogPacket.getTopmostViaHost();
                int viaPort = capturedLogPacket.getTopmostViaPort();
                if (viaPort == -1) {
                    viaPort = 5060;
                }
                EmulatedEndpoint sourceEndpoint = SipTester.getEndpoint(viaAddress, viaPort);
                 if (sourceEndpoint != null) {
                    sourceEndpoint.addOriginatingPacket(capturedLogPacket);
                    logger.debug("sourceEndpoint " + sourceEndpoint.getIpAddress() + ":" + sourceEndpoint.getPort());
                } else {
                    logger.debug("Source Endpoint " + viaAddress + ":" + viaPort + " is not emulated");
                }
            } else {
                SipResponse sipResponse = new SipResponse((SIPResponse) capturedLogPacket
                        .getSipPacket(), capturedLogPacket.getTimeStamp(), capturedLogPacket.getFrameId());

                String transactionId = capturedLogPacket.getTransactionId();
                logger.debug("checking sipResponse " + capturedLogPacket.getSipPacket().getFirstLine()+ " transactionId " + transactionId);
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
