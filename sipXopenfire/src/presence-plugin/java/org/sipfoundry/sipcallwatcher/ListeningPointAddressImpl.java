/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher;

import org.sipfoundry.commons.jainsip.ListeningPointAddress;



public class ListeningPointAddressImpl extends ListeningPointAddress {
    
    private String transport;

    public ListeningPointAddressImpl(String transport) {
        this.transport = transport;
    }

    @Override
    public String getHost() {
       return CallWatcher.getConfig().getWatcherAddress();
    }

    @Override
    public int getPort() {
        return CallWatcher.getConfig().getWatcherPort();
    }

    @Override
    public String getTransport() {
        return this.transport;
    }

}
