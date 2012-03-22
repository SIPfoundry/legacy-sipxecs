/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import org.sipfoundry.commons.jainsip.ListeningPointAddress;

public class ListeningPointAddressImpl extends ListeningPointAddress {
    private String transport;

    public ListeningPointAddressImpl(String transport) {
        this.transport = transport;
    }

    @Override
    public String getHost() {
       return RestServer.getRestServerConfig().getIpAddress();
    }

    @Override
    public int getPort() {
        return RestServer.getRestServerConfig().getSipPort();
        
    }

    @Override
    public String getTransport() {
       return transport;
    }

}
