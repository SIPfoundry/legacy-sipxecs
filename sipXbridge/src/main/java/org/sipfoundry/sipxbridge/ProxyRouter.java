/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.stack.DefaultRouter;

import java.util.LinkedList;
import java.util.ListIterator;

import javax.sip.SipException;
import javax.sip.SipStack;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;
import javax.sip.message.Request;

/**
 * Proxy router class. Resolves a hop to the sipx proxy.
 * Request URI is the domain name of the proxy.
 * 
 * @author M. Ranganathan
 *
 */
public class ProxyRouter extends DefaultRouter {

    public ProxyRouter(SipStack sipStack, String outboundProxy) {
        super(sipStack, outboundProxy);

    }

    @Override
    public Hop getNextHop(Request request) throws SipException {

         if (((SipURI) request.getRequestURI()).getHost().equals(Gateway.getSipxProxyDomain())) {
            return Gateway.getSipxProxyHop();
        } else {
            return super.getNextHop(request);
        }

    }

    @Override
    public ListIterator getNextHops(Request request) {
        try {
            LinkedList<Hop> retval = new LinkedList<Hop>();
            retval.add(getNextHop(request));
            return retval.listIterator();
        } catch (Exception ex) {
            return new LinkedList<Hop>().listIterator();
        }

    }

}
