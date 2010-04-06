/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.siptester;

import gov.nist.javax.sip.message.SIPRequest;

public abstract class SipTransaction {
    
    protected SipRequest sipRequest;
    
    protected static long minDelay = Long.MAX_VALUE;
    
    protected long delay;
    
    
    public long getDelay() {
        return this.delay;
    }

    public long getTime() {
        return this.sipRequest.getTime();
    }
    
    /**
     * @return the sipRequest
     */
    public SipRequest getSipRequest() {
        return sipRequest;
    }
    
    public String getTransactionId() {
        return ((SIPRequest)sipRequest.getSipRequest()).getTransactionId();
    }
    
    
}
