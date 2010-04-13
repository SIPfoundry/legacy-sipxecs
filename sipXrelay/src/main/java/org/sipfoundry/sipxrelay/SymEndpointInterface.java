/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

/**
 * A sym endpoint is a host-port pair. It can be assigned the role of either a
 * reciever or a sender.
 * 
 * @author M. Ranganathan
 * 
 */
public interface SymEndpointInterface {

    /**
     * Get the local or remote port where data is received or the remote port
     * where data is sent.
     * 
     * @return -- the port where data is received or sent
     * 
     */
    public int getPort();

    /**
     * Get the IP address ( local or remote ) where data is received or sent.
     * 
     * @return -- the local or remote Ip address.
     */
    public String getIpAddress();

   

}
