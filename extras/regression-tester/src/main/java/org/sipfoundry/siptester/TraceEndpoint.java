package org.sipfoundry.siptester;

import java.util.Collection;
import java.util.HashSet;

import org.sipfoundry.commons.jainsip.ListeningPointAddress;

/**
 * This class represents a endpoint from the SUT where the trace was originally taken.
 * 
 */
public class TraceEndpoint extends HostPort  {
    private EmulatedEndpoint emulatedEndpoint;
    private String defaultTransport = "udp";
    private Behavior behavior;
    private int emulatedPort;
    private boolean isEmulated = true;
    
    public String getDefaultTransport() {
        return this.defaultTransport;
    }
    
    public void setDefaultTransport(String transport) {
        this.defaultTransport = transport;
    }
    
    /**
     * @param ipAddress the ipAddress to set
     */
    public void setIpAddress(String ipAddress) {
        this.ipAddress = ipAddress;
    }
    /**
     * @return the ipAddress
     */
    public String getIpAddress() {
        return ipAddress;
    }
    /**
     * @param port the port to set
     */
    public void setPort(String port) {  
        this.port = Integer.parseInt(port) ;
    }
  
    
    public void setEmulatedPort(String port) {
        this.emulatedPort = Integer.parseInt(port) + SipTester.getTesterConfig().getBasePort();
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
   
      
}
