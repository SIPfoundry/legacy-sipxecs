/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight.discovery;

import java.util.ListIterator;

import javax.sip.*;
import javax.sip.header.*;
import javax.sip.message.*;

import org.sipfoundry.ao.*;

/**
 * [Enter descriptive text here]
 * <p>
 * 
 * @author Mardy Marshall
 */
public class DiscoveryAgentImpl extends ActiveObject implements DiscoveryAgent {
    private final String targetAddress;
    private final DiscoveryService discoveryService;
    private final SipProvider sipProvider;
    private boolean terminated = false;

    public DiscoveryAgentImpl(String targetAddress, DiscoveryService discoveryService) {
        this.targetAddress = targetAddress;
        this.discoveryService = discoveryService;
        this.sipProvider = discoveryService.getSipProvider();
    }

    @Startup
    public void discover() {
        try {
            // if (targetAddress.isReachable(10 * 1000)) {
            if (true) {
                ClientTransaction clientTransaction;

                Request request = discoveryService.createOptionsRequest(targetAddress, 5060);
                // Create the client transaction.
                clientTransaction = sipProvider.getNewClientTransaction(request);
                clientTransaction.setApplicationData(this);
                // send the request out.
                clientTransaction.sendRequest();
            }
//        } catch (IOException e) {
//            e.printStackTrace();
        } catch (TransactionUnavailableException e) {
            e.printStackTrace();
        } catch (SipException e) {
            e.printStackTrace();
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.preflight.discovery.DiscoveryAgent#processResponse(javax.sip.ResponseEvent)
     */
    @SuppressWarnings("unchecked")
    @Synchronous
    public void processResponse(ResponseEvent responseEvent) {
        String userAgentInfo = "";
        Response response = (Response) responseEvent.getResponse();
        UserAgentHeader userAgentHeader = (UserAgentHeader) response.getHeader(UserAgentHeader.NAME);
        if (userAgentHeader != null) {
            ListIterator<String> headerValues = userAgentHeader.getProduct();
            while (headerValues.hasNext()) {
                userAgentInfo += headerValues.next();
                if (headerValues.hasNext()) {
                    userAgentInfo += ", ";
                }
            }
        }
        
        discoveryService.addDiscovered(targetAddress, userAgentInfo);
        ActiveObjectGroup activeObjectGroup = getActiveObjectGroup();
        activeObjectGroup.deleteInstance(this);
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.preflight.discovery.DiscoveryAgent#processTimeout(javax.sip.TimeoutEvent)
     */
    @Synchronous
    public void processTimeout(TimeoutEvent timeoutEvent) {
        if (!terminated) {
            discoveryService.addDiscovered(targetAddress, "");
        	ActiveObjectGroup activeObjectGroup = getActiveObjectGroup();
        	activeObjectGroup.deleteInstance(this);
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.preflight.discovery.DiscoveryAgent#processTransactionTerminated(javax.sip.TransactionTerminatedEvent)
     */
    @Synchronous
    public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedEvent) {
    }

    @Terminate
    @Synchronous
    public void terminate() {
        terminated = true;
        ActiveObjectGroup activeObjectGroup = getActiveObjectGroup();
        activeObjectGroup.deleteInstance(this);
    }
}
