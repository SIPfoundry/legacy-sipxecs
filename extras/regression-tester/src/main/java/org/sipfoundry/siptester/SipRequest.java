package org.sipfoundry.siptester;

import gov.nist.javax.sip.message.MessageExt;
import gov.nist.javax.sip.message.RequestExt;

import java.io.File;

public class SipRequest extends SipMessage {
    
    private RequestExt sipRequest;
    
    
    public SipRequest(RequestExt sipRequest, long time, File logFile) {
        this.sipRequest = sipRequest;
        this.time = time;
        this.logFile = logFile;
    }

    
    /**
     * @return the sipRequest
     */
    public RequestExt getSipRequest() {
        return sipRequest;
    }
    
    public MessageExt getSipMessage() {
        return sipRequest;
    }


  

}
