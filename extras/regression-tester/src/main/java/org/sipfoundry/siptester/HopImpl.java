package org.sipfoundry.siptester;

import javax.sip.address.Hop;

public class HopImpl implements Hop {
    
    String host;
    String transport;
    int port;
    public HopImpl (String host, int port, String transport) {
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
