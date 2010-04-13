/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.siptester;

import java.util.Collection;
import java.util.HashSet;

import org.sipfoundry.commons.jainsip.ListeningPointAddress;

/**
 * This class represents a endpoint from the SUT where the trace was originally taken.
 * 
 */
public class TraceEndpoint   {
    private EmulatedEndpoint emulatedEndpoint;
    private String defaultTransport = "udp";
    private Behavior behavior;
    private int emulatedPort;
    private boolean isEmulated = true;
    
    protected HashSet<HostPort> traceIpAddresses = new HashSet<HostPort>();
   
    
    public HashSet<HostPort> getTraceIpAddresses() {
        return this.traceIpAddresses;
    }
    
    public void addHostPort(String hostPort) {
        int port = 5060;
        String host = hostPort;
        if (hostPort.indexOf(":") > 0 ) {
            port = Integer.parseInt(hostPort.substring(hostPort.indexOf(":") + 1));
            host = hostPort.substring(0,hostPort.indexOf(":"));
        }
        this.traceIpAddresses.add( new HostPort(host,port));
       
    }
    
    
    @Override
    public int hashCode() {
       return traceIpAddresses.hashCode();
    }
    
    public String toString() {
        return traceIpAddresses.toString();
    }
    
    @Override
    public boolean equals(Object that) {
        TraceEndpoint other = (TraceEndpoint) that;
        return this.traceIpAddresses.equals(other.traceIpAddresses);
    }
   
    
    public String getDefaultTransport() {
        return this.defaultTransport;
    }
    
    public void setDefaultTransport(String transport) {
        this.defaultTransport = transport;
    }
    
    /**
     * @param ipAddress the ipAddress to set
     */
    public void addTraceHostPort(HostPort hostPort) {
       this.traceIpAddresses.add(hostPort);
    }
    
    
    
    public void setEmulatedEndpoint(EmulatedEndpoint endpoint) {
        this.emulatedEndpoint = endpoint;
        
    }
    public EmulatedEndpoint getEmulatedEndpoint() {
       return this.emulatedEndpoint;
    }
    
    public void setBehavior(String behavior) {
        this.behavior = Behavior.valueOf(behavior);
    }
    
    public void setIsEmulated(String bool) {
        this.isEmulated = Boolean.parseBoolean(bool);
    }
    
    public boolean isEmulated() {
        return this.isEmulated;
    }
    
    public Behavior getBehavior() {
        return this.behavior;
    }

    public int getEmulatedPort() {
        return this.emulatedPort;
    }
    
    public void setEmulatedPort(String portString) {
        this.emulatedPort =  SipTester.getTesterConfig().getBasePort() + Integer.parseInt(portString);
    }
   
      
}
