/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.siptester;

import gov.nist.javax.sip.message.MessageExt;
import gov.nist.javax.sip.message.ResponseExt;
import gov.nist.javax.sip.message.SIPResponse;

import java.io.File;
import java.util.Collection;
import java.util.HashSet;
import java.util.concurrent.Semaphore;

import javax.sip.ResponseEvent;

import org.apache.log4j.Logger;

public class SipResponse extends SipMessage {
     private gov.nist.javax.sip.message.ResponseExt sipResponse;
     private ResponseEvent responseEvent;  
     private Semaphore preconditionSem = new Semaphore(0);
     private static Logger logger = Logger.getLogger(SipResponse.class);
     private HashSet<SipMessage> activatedBy = new HashSet<SipMessage>();
     int permits = 0;
     
     private HashSet<SipResponse> activateOnSendResponse = new HashSet<SipResponse>();
     
     public void waitForPrecondition() {
         try {
             logger.debug("Frame = " + this.getFrameId() + " line = " + 
                         this.getSipResponse().getFirstLine() +  " waiting for " + permits );
             preconditionSem.acquire(permits);
         } catch (Exception ex) {
             SipTester.fail("unexpected exception",ex);
         }
     }
     
    
    public SipResponse( ResponseExt sipResponse, long time, String frameId) {
        this.sipResponse = sipResponse;
        this.time = time;
        this.setFrameId(frameId);
    }

    public void addPermit() {
        this.permits++;
        logger.debug("Frame = " + this.getFrameId() + " addPermit " + this.permits);
        
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

    public void triggerPreCondition(SipResponse sipResponse) {
        logger.debug("triggerPerCondition: Frame = " + this.getFrameId() + " npermits  " + 
        		this.preconditionSem.availablePermits());
        this.preconditionSem.release();
        //this.permits --;
        this.activatedBy.remove(sipResponse);
    }


	public void addPostCondition(SipResponse response) {
		logger.debug("frame " + this.getFrameId () + " addPostCondition " + response.getFrameId());
		response.addPermit();
		this.activateOnSendResponse.add(response);
		response.addActivatedBy(this);
	}


	void addActivatedBy(SipMessage sipMessage) {
		this.activatedBy.add(sipMessage);
		
	}


	public void firePermits() {
		for (SipResponse response : this.activateOnSendResponse ) {
			response.triggerPreCondition(this);
		}	
	}
	
	public HashSet<SipMessage> getActivatedBy() {
		return this.activatedBy;
	}


	public void triggerPreCondition(SipRequest sipRequest) {
		   logger.debug("triggerPerCondition: Frame = " + this.getFrameId() + " npermits  " + 
	        		this.preconditionSem.availablePermits());
	        this.preconditionSem.release();
	        
	        this.activatedBy.remove(sipRequest);
		
	}


    
   

}
