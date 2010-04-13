/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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
