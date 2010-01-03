package org.sipfoundry.siptester;

import gov.nist.javax.sip.message.MessageExt;
import gov.nist.javax.sip.message.ResponseExt;
import gov.nist.javax.sip.message.SIPResponse;

import java.io.File;
import java.util.Collection;
import java.util.HashSet;

import javax.sip.ResponseEvent;

public class SipResponse extends SipMessage {
     private gov.nist.javax.sip.message.ResponseExt sipResponse;
     
     private ResponseEvent responseEvent;
     
    
    public SipResponse( ResponseExt sipResponse, long time, String frameId) {
        this.sipResponse = sipResponse;
        this.time = time;
        this.setFrameId(frameId);
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

    public int getStatusCode() {
       return sipResponse.getStatusCode();
    }

    /**
     * @param responseEvent the responseEvent to set
     */
    public void setResponseEvent(ResponseEvent responseEvent) {
        this.responseEvent = responseEvent;
        super.dialog = responseEvent.getDialog();
    }

    /**
     * @return the responseEvent
     */
    public ResponseEvent getResponseEvent() {
        return responseEvent;
    }
   

}
