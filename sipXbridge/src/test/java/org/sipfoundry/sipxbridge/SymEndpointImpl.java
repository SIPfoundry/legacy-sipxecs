package org.sipfoundry.sipxbridge;

import org.sipfoundry.sipxbridge.symmitron.SymEndpointInterface;

class SymEndpointImpl implements SymEndpointInterface {
    String id;
    String ipAddress;
    int port;

    public String getId() {
        
        return id;
    }

    public String getIpAddress() {
      
        return ipAddress;
    }

    public int getPort() {
        
        return port;
    }

}
