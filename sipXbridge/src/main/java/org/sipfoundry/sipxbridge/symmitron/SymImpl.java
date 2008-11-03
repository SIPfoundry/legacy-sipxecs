package org.sipfoundry.sipxbridge.symmitron;
/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

import java.util.Map;

import org.sipfoundry.sipxbridge.symmitron.SymEndpointInterface;
import org.sipfoundry.sipxbridge.symmitron.SymInterface;


public class SymImpl implements SymInterface {
    String id;
    SymmitronClient symmitronClient;
    
    private SymEndpointInterface receiver;
    
    private SymEndpointInterface transmitter;
    
 
    public SymImpl () {
        
    }
    
    public SymImpl(String id ) {
        this.id = id;
    }
    
    protected SymImpl( String id, SymmitronClient client   ) {
        this(id);
        this.symmitronClient = client;
        
    }

    public String getId() {
        
        return id;
    }

    public SymEndpointInterface getReceiver() {
       
        return receiver;
    }

    public SymEndpointInterface getTransmitter() {
       
        return transmitter;
    }

    /**
     * @param receiver the receiver to set
     */
    public void setReceiver(SymEndpointInterface receiver) {
        this.receiver = receiver;
    }

    /**
     * @param transmitter the transmitter to set
     */
    public void setTransmitter(SymEndpointInterface transmitter) {
        this.transmitter = transmitter;
    }

    public String getRecieverState() {
      return this.symmitronClient.getReceiverState(id);
    }

  
}
