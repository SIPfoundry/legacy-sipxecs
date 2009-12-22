package org.sipfoundry.siptester;

import org.sipfoundry.commons.jainsip.ListeningPointAddress;

public class ListeningPointAddressImpl extends ListeningPointAddress {

    SipTesterConfig sipTesterConfg;
    String transport;
    Endpoint endpoint ;
    
    public ListeningPointAddressImpl( Endpoint endpoint, String transport) {
        this.endpoint = endpoint;
        this.transport = transport;
    }
    
    
    @Override
    public String getHost() {
        return endpoint.getIpAddress();
    }

    @Override
    public int getPort() {
      return endpoint.getPort();
    }

    @Override
    public String getTransport() {
         return this.transport;
    }

}
