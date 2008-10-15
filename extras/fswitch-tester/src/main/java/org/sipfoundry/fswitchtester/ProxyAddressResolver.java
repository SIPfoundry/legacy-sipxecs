package org.sipfoundry.fswitchtester;

import gov.nist.core.net.AddressResolver;

import javax.sip.address.Hop;


/**
 * The Address resolver to resolve proxy domain to a hop to the outbound proxy
 * server.
 * 
 * @author M. Ranganathan
 * 
 */
public class ProxyAddressResolver implements AddressResolver {
   
    public Hop resolveAddress(Hop hop) {

        if (hop.getHost().equals(FreeSwitchTester.getSipxProxyDomain())) {
            return FreeSwitchTester.getSipxProxyHop();
        } else {
            return hop;
        }
    }

}

