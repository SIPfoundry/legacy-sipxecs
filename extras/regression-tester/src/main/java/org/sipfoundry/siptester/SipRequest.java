package org.sipfoundry.siptester;

import gov.nist.javax.sip.message.MessageExt;
import gov.nist.javax.sip.message.RequestExt;

import java.io.File;

import javax.sip.RequestEvent;

public class SipRequest extends SipMessage {
    
    private RequestExt sipRequest;
    
    private RequestEvent requestEvent;
    
    
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


    /**
     * @param requestEvent the requestEvent to set
     */
    public void setRequestEvent(RequestEvent requestEvent) {
        this.requestEvent = requestEvent;
    }


    /**
     * @return the requestEvent
     */
    public RequestEvent getRequestEvent() {
        return requestEvent;
    }


  

}
