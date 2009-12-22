package org.sipfoundry.siptester;

import gov.nist.javax.sip.message.MessageExt;
import gov.nist.javax.sip.message.ResponseExt;
import gov.nist.javax.sip.message.SIPResponse;

import java.io.File;
import java.util.Collection;
import java.util.HashSet;

public class SipResponse extends SipMessage {
     private gov.nist.javax.sip.message.ResponseExt sipResponse;
     
     private HashSet<SipClientTransaction> postCondition = new HashSet<SipClientTransaction>();
    
    public SipResponse( ResponseExt sipResponse, long time, File logFile) {
        this.sipResponse = sipResponse;
        this.time = time;
        this.logFile = logFile;
    }

    /**
     * @return the sipRequest
     */
    public gov.nist.javax.sip.message.ResponseExt getSipResponse() {
        return sipResponse;
    }

    
    public MessageExt getSipMessage() {
        return (MessageExt)sipResponse;
    }

    public void addPostCondition(SipClientTransaction previousTx) {
        this.postCondition.add(previousTx);
    
      
    }



    public Collection<SipClientTransaction> getPostConditions() {
        return this.postCondition;
    }

   

}
