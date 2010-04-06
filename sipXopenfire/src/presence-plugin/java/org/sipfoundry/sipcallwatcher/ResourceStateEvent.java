/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher;

import java.util.EventObject;
import org.sipfoundry.sipcallwatcher.DialogInfoMessagePart.EndpointInfo;

public class ResourceStateEvent extends EventObject {
    private static final long serialVersionUID = -7228675062866489237L;
    private SipResourceState state;
    private String user ;
    private EndpointInfo remoteEndpoint;
    
    public ResourceStateEvent(Object source) {
        super(source);
    }
    
    public ResourceStateEvent (Subscriber subscriber, String user, SipResourceState newState, EndpointInfo remoteEndpoint) {
        this(subscriber);
        this.state = newState;
        this.user = user;
        this.remoteEndpoint = remoteEndpoint;
    }

    /**
     * @return the state
     */
    public SipResourceState getState() {
        return state;
    }

    /**
     * @param user the user to set
     */
    public void setUser(String user) {
        this.user = user;
    }

    /**
     * @return the user
     */
    public String getUser() {
        return user;
    }

    public EndpointInfo getRemoteEndpoint()
    {
        return remoteEndpoint;
    }

    public void setRemoteEndpoint( EndpointInfo remoteEndpoint )
    {
        this.remoteEndpoint = remoteEndpoint;
    }




}
