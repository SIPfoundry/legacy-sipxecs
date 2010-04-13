/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

import java.util.EventObject;
import java.util.Hashtable;

import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;

public class SipListenerImpl implements SipListener {

    private Hashtable<SipProvider, ConferenceTestClient> testerClients = new Hashtable<SipProvider, ConferenceTestClient>();

    private ConferenceTestClient getClient(EventObject event) {
        
        SipProvider provider = ( SipProvider ) event.getSource();
        return testerClients.get(provider);
        
    }
    
    public SipListenerImpl() {

    }

    public void addSlave(SipProvider provider, ConferenceTestClient client) {
        this.testerClients.put(provider, client);

    }

    public void processDialogTerminated(DialogTerminatedEvent dte) {
        getClient(dte).processDialogTerminated(dte);

    }

    public void processIOException(IOExceptionEvent arg0) {

    }

    public void processRequest(RequestEvent requestEvent) {
       getClient(requestEvent).processRequest(requestEvent);
    }

    public void processResponse(ResponseEvent re) {
       getClient(re).processResponse(re);

    }

    public void processTimeout(TimeoutEvent te) {
        throw new RuntimeException("Unexpected event - timeout");

    }

    public void processTransactionTerminated(TransactionTerminatedEvent tte) {
       

    }

}
