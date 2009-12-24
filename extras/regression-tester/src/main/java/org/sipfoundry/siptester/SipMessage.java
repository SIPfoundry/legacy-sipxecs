package org.sipfoundry.siptester;

import gov.nist.javax.sip.message.MessageExt;

import java.io.File;
import java.util.Collection;
import java.util.HashSet;


public abstract class SipMessage implements Comparable<SipMessage> {
    protected File logFile;
    protected long time;   
    private HashSet<SipClientTransaction> postCondition = new HashSet<SipClientTransaction>();
    
    public void addPostCondition(SipClientTransaction previousTx) {
        this.postCondition.add(previousTx);      
    }
    
    public Collection<SipClientTransaction> getPostConditions() {
        return this.postCondition;
    }
    
    
    @Override
    public int compareTo(SipMessage sipMessage) {
       if ( sipMessage == null ) throw new NullPointerException("Compare with null ");
       if ( this.time < sipMessage.time) return -1;
       else if (this.time == sipMessage.time) return 0;
       else return 1;
    }
    
    /**
     * @return the logFile
     */
    public File getLogFile() {
        return logFile;
    }
    
    public long getTime() {
        return time;
    }
    
    @Override 
    public int hashCode() {
        return getSipMessage().getTopmostViaHeader().getBranch().hashCode();
    }
    public abstract MessageExt getSipMessage();
    
}
