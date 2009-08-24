package org.sipfoundry.commons.jainsip;

import javax.sip.ListeningPoint;
import javax.sip.SipProvider;

/**
 * Passed back to the stack for creation of listening points.
 */
public abstract class ListeningPointAddress {
    SipProvider sipProvider;
    ListeningPoint listeningPoint;
    
    public SipProvider getSipProvider() {
        return sipProvider;
    }
    
    public ListeningPoint getListeningPoint() {
        return this.listeningPoint;
    }
    public abstract String getHost();
    public abstract int getPort();
    public abstract String getTransport();

}
