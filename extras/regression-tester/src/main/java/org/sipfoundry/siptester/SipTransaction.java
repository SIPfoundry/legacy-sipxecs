package org.sipfoundry.siptester;

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
    
}
