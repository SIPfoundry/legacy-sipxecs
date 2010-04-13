/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.siprouter;

import gov.nist.javax.sip.SipStackImpl;
import gov.nist.javax.sip.stack.DefaultRouter;

import java.util.LinkedList;
import java.util.ListIterator;

import javax.sip.SipException;
import javax.sip.SipStack;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;
import javax.sip.header.RouteHeader;
import javax.sip.message.Request;

import org.apache.log4j.Logger;


public class ProxyRouter extends DefaultRouter {
	Logger LOG;
	FindSipServer finder;
	
	public ProxyRouter(SipStack sipStack, String outboundProxy) {
        super(sipStack, outboundProxy);
    	LOG = Logger.getLogger(((SipStackImpl) sipStack).getStackLogger().getLoggerName());
    	finder = new FindSipServer(LOG);
    }

    @Override
    public Hop getNextHop(Request request) throws SipException {
        /*
         * Request has a Route header defined - then get the next hop
         * based on the route header.
         */
    	SipURI uri = (SipURI)request.getRequestURI();
    	LOG.debug(String.format("ProxyRouter::getNextHop Need to lookup %s for inbound %s ", uri.toString() , request.getMethod()));
    	    
        if ( request.getHeader(RouteHeader.NAME) != null  || uri.getMAddrParam() != null ) {
            
            Hop nextHop =  super.getNextHop(request);   
            return nextHop;
        }

    	LOG.debug(String.format("ProxyRouter::getNextHop Need to lookup %s ", uri.toString()));
    	Hop h = finder.findServer(uri) ;
    	if (h == null) {
    		LOG.debug(String.format("ProxyRouter::getNextHop could not find next hop for %s", uri.toString()));
    		return super.getNextHop(request) ;
    	}
		LOG.debug(String.format("ProxyRouter::getNextHop next hop for %s is %s", uri.toString(), h.toString()));
    	return h;
    }

    @Override
    public ListIterator<Hop> getNextHops(Request request) {
        try {
            LinkedList<Hop> retval = new LinkedList<Hop>();
            retval.add(getNextHop(request));
            return retval.listIterator();
        } catch (Exception ex) {
            return new LinkedList<Hop>().listIterator();
        }

    }

}
