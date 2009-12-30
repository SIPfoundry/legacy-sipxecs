/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.siptester;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.ResponseExt;

import java.util.Collection;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import javax.sip.message.Message;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

public class SipDialog {
   
    String fromTag;
    String toTag;
    private DialogExt dialog;
    private Response lastResponse;
    private RequestExt lastRequestReceived;

    Semaphore prackSem = new Semaphore(0);
    Semaphore ackSem = new Semaphore(0);

    Collection<SipClientTransaction> clientTransactions = new ConcurrentSkipListSet<SipClientTransaction>();

    Collection<SipServerTransaction> serverTransactions = new ConcurrentSkipListSet<SipServerTransaction>();
    
    
    private static Logger logger = Logger.getLogger(SipDialog.class);
    
    public SipDialog() {

    }

    public void addSipClientTransaction(SipClientTransaction sipClientTransaction) {
        this.clientTransactions.add(sipClientTransaction); 
    }

    public void addSipServerTransaction(SipServerTransaction sipServerTransaction) {
        this.serverTransactions.add(sipServerTransaction);
        sipServerTransaction.setDialog(this);
        
    }

    /**
     * @param dialog the dialog to set
     */
    public void setDialog(DialogExt dialog) {
        if ( dialog == null ) {
            logger.debug("setDialog: setting dialog to null");
        }
        this.dialog = dialog;
    }

    /**
     * @return the dialog
     */
    public DialogExt getDialog() {
        return dialog;
    }

    public Response getLastResponse() {
        return this.lastResponse;
    }

    public void waitForOk() {
        try {
            boolean acquired = this.ackSem.tryAcquire(10, TimeUnit.SECONDS);
            if (!acquired) {
                SipTester.fail("Could not acuqire ACK semaphore");
            }
        } catch (Exception ex) {
            SipTester.fail("Unexpected exception ", ex);
        }
    }

    public void setLastResponse(Response response) {
        this.lastResponse = response;
        if (response.getStatusCode() == Response.OK
                && SipUtilities.getCSeqMethod(response).equals(Request.INVITE)) {
            this.ackSem.release();
        }
    }
    
    public void setLastRequestReceived(RequestExt request) {
        this.lastRequestReceived = request;
        if ( this.lastRequestReceived.getMethod().equals(Request.PRACK) ) {
            System.out.println("releaseParak sem " + this);
            this.prackSem.release();
        }
    }

    public void waitForPrack() {
        try {
            boolean acquired = this.prackSem.tryAcquire(10, TimeUnit.SECONDS);
            if (!acquired) {
                SipTester.fail("Could not acuqire PRACK semaphore " + this);
            }
        } catch (Exception ex) {
            SipTester.fail("Unexpected exception ", ex);
        }
        
    }
}
