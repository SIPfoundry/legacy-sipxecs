/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.net.InetAddress;

import javax.sip.address.Hop;

import org.apache.log4j.Logger;

import gov.nist.core.net.AddressResolver;

/**
 * The Address resolver to resolve proxy domain to a hop to the outbound proxy
 * server.
 * 
 * @author M. Ranganathan
 * 
 */
public class ProxyAddressResolver implements AddressResolver {
    private static Logger logger = Logger.getLogger(ProxyAddressResolver.class);
   
    private static AccountManagerImpl accountManager = Gateway
            .getAccountManager();

    public Hop resolveAddress(Hop hop) {
        if ( logger.isDebugEnabled()) {
            logger.debug("resolveAddress " + hop.getHost());
        }
        if ( hop.getHost().equals("127.0.0.1")) { 
            return hop; // For testing only.
        } else if (hop.getHost().equals(Gateway.getSipxProxyDomain())) {
            return Gateway.getSipxProxyHop();
        } else if (Gateway.getAccountManager().getHopToItsp(hop.getHost()) != null) {
            return Gateway.getAccountManager().getHopToItsp(hop.getHost());
        } else
            return hop;
    }

}
