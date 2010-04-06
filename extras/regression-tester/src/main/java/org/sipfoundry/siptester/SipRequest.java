/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.siptester;

import gov.nist.javax.sip.message.MessageExt;
import gov.nist.javax.sip.message.RequestExt;

import java.io.File;
import java.util.HashSet;

import javax.sip.RequestEvent;

public class SipRequest extends SipMessage {
    
    private RequestExt sipRequest;
    
    private RequestEvent requestEvent;

    private HostPort hostPort;
   
    
    
    public SipRequest(RequestExt sipRequest, long time, String frameId) {
        this.sipRequest = sipRequest;
        this.time = time;
        this.setFrameId(frameId);
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
        super.dialog = requestEvent.getDialog();
    }


    /**
     * @return the requestEvent
     */
    public RequestEvent getRequestEvent() {
        return requestEvent;
    }
    
    public void setTargetHostPort(HostPort hostPort) {
        this.hostPort = hostPort;
    }
    
    public HostPort getTargetHostPort() {
        return this.hostPort;
    }




	public HostPort getSourceHostPort() {
		String host = this.getSipRequest().getTopmostViaHeader().getHost();
		int port = this.getSipRequest().getTopmostViaHeader().getPort();
		if (port == -1 ) port = 5060;
	
		return new HostPort(host,port);
	}

  

}
