package org.sipfoundry.siptester;

import java.util.Collection;
import java.util.HashSet;

import org.sipfoundry.commons.jainsip.ListeningPointAddress;

/**
 * This class represents a endpoint from the SUT where the trace was originally taken.
 * 
 */
public class SutUA  {
    private String ipAddress;
    private int    port;
    private Endpoint endpoint;
    HashSet<String> registrations = new HashSet<String>();
    private String defaultTransport = "udp";
    
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
        this.port = Integer.parseInt(port);
    }
    /**
     * @return the port
     */
    public int getPort() {
        return port;
    }
    
    public void setEndpoint(Endpoint endpoint) {
        this.endpoint = endpoint;
    }
    public Endpoint getEndpoint() {
       return this.endpoint;
    }
    
    /**
     * @param name the name to set
     */
    public void addRegistration(String name) {
        this.registrations.add(name);
    }
    public Collection<String> getRegistrations() {
        return this.registrations;
    }
      
}
