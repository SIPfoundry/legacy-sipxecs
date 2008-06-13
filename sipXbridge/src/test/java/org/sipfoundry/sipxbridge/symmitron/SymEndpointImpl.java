package org.sipfoundry.sipxbridge.symmitron;

import org.sipfoundry.sipxbridge.symmitron.SymEndpointInterface;

public class SymEndpointImpl implements SymEndpointInterface {
    private String id;
    private String ipAddress;
    private int port;

    public String getId() {
        
        return id;
    }

    public String getIpAddress() {
      
        return ipAddress;
    }

    public int getPort() {
        
        return port;
    }

    /**
     * @param ipAddress the ipAddress to set
     */
    public void setIpAddress(String ipAddress) {
        this.ipAddress = ipAddress;
    }

    /**
     * @param port the port to set
     */
    public void setPort(int port) {
        this.port = port;
    }

    /**
     * @param id the id to set
     */
    public void setId(String id) {
        this.id = id;
    }

}
