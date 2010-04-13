/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.siptester;

import org.sipfoundry.commons.jainsip.ListeningPointAddress;

public class ListeningPointAddressImpl extends ListeningPointAddress {

    SipTesterConfig sipTesterConfg;
    String transport;
    EmulatedEndpoint endpoint ;
    
    public ListeningPointAddressImpl( EmulatedEndpoint endpoint, String transport) {
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
