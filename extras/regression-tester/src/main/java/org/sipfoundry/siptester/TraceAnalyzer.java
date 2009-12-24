package org.sipfoundry.siptester;

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
        Collection<CapturedLogPacket> capturedLogPackets = new LogFileParser().parse("file:" + mergedFileName);
        Iterator<CapturedLogPacket> it = capturedLogPackets.iterator();
        logger.debug("found " + capturedLogPackets.size() + " packets " );
        while ( it.hasNext() ) {
            CapturedLogPacket capturedLogPacket = it.next();
            if ( capturedLogPacket.getDestinationPort() == 0 ) {
                HostPort hostPort = CapturedLogPacket.hostMapper.get(capturedLogPacket.getDestinationAddess());
                if ( hostPort != null ) {  
                    capturedLogPacket.setDestinationAddress(hostPort.getHost());
                    capturedLogPacket.setDestinationPort(hostPort.getPort());
                }
            }
            logger.debug("destAddress = " + capturedLogPacket.getDestinationAddess() + " src " + capturedLogPacket.getSourceAddress());
            
            if (capturedLogPacket.getSipPacket() instanceof Request) {
                String address = capturedLogPacket.getDestinationAddess();
                int port = capturedLogPacket.getDestinationPort();
                
                
                Endpoint destEndpoint = SipTester.getEndpoint(address, port);
                if (destEndpoint != null) {
                    destEndpoint.addReceivedPacket(capturedLogPacket);
                }
                String viaAddress = capturedLogPacket.getTopmostViaHost();
                int  viaPort = capturedLogPacket.getTopmostViaPort();
                if ( viaPort == -1 ) {
                    viaPort = 5060;
                }
                Endpoint sourceEndpoint = SipTester.getEndpoint(viaAddress,viaPort);
                if ( sourceEndpoint != null ) {
                    System.out.println("request " + capturedLogPacket.getMessage().getFirstLine() );
                    sourceEndpoint.addOriginatingPacket(capturedLogPacket);
                }
            } else {
                String transactionId = capturedLogPacket.getTransactionId();
                for (Endpoint endpoint : SipTester.getEndpoints()) {
                    /*
                     * Find the server tx to which this response belongs.
                     */
                    if (SipTester.getSipServerTransaction(transactionId) != null) {
                        logger.trace("found SipServerTransaction for message OUTGOING "
                                + transactionId);
                        /*
                         * Add the response to the server tx.
                         */
                        endpoint.addOriginatingPacket(capturedLogPacket);
                    }

                    if (SipTester.getSipClientTransaction(transactionId) != null) {
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
