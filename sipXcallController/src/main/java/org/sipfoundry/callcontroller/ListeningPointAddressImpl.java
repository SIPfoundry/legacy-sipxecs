package org.sipfoundry.callcontroller;

import org.sipfoundry.commons.jainsip.ListeningPointAddress;

public class ListeningPointAddressImpl extends ListeningPointAddress {
    private String host;
    private int port;
    private String transport;
    
    public ListeningPointAddressImpl( String host, int port, String transport) {
        this.host = host;
        this.port = port;
        this.transport = transport;
    }

    @Override
    public String getHost() {
       
        return host;
    }

    @Override
    public int getPort() {
       
        return port;
    }

    @Override
    public String getTransport() {
       
        return transport;
    }

}
