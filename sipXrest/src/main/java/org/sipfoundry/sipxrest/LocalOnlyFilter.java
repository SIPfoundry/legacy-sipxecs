/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import java.net.InetAddress;
import java.util.Collection;

import javax.sip.address.Hop;
import javax.sip.address.SipURI;

import org.apache.log4j.Logger;
import org.restlet.Filter;
import org.restlet.data.MediaType;
import org.restlet.data.Protocol;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.sipfoundry.commons.siprouter.FindSipServer;

public class LocalOnlyFilter extends Filter {
    
    private static final Logger logger = Logger.getLogger(LocalOnlyFilter.class);
    

    @Override
    protected int beforeHandle(Request request, Response response) {
        String remoteAddr = request.getClientInfo().getAddress();
        int httpPort = request.getHostRef().getHostPort();
        try {
            String proxyDomain = RestServer.getRestServerConfig().getSipxProxyDomain();
            SipURI sipUri = RestServer.getAddressFactory().createSipURI(null, proxyDomain);

            logger.debug("Authentication request " + remoteAddr);
            Collection<Hop> hops = new FindSipServer(logger).getSipxProxyAddresses(sipUri);

            for (Hop hop : hops) {
                if (InetAddress.getByName(hop.getHost()).getHostAddress().equals(remoteAddr)) {
                    if (httpPort != RestServer.getRestServerConfig().getHttpPort()) {
                        return Filter.STOP;
                    }
                    if (!request.getProtocol().equals(Protocol.HTTPS)) {
                        logger.debug("Request from Proxy must be over HTTPS ");
                        return Filter.STOP;
                    }

                    logger.debug("Request from sipx domain");
                    return Filter.CONTINUE;
                }
            }
            logger.debug("Request not from sipx domain -- rejecting");
            response.setStatus(Status.CLIENT_ERROR_FORBIDDEN);        
            response.setEntity("Only Local access allowed", MediaType.TEXT_PLAIN);
            return Filter.STOP;
        } catch (Exception ex) {
            response.setEntity("Internal Error", MediaType.TEXT_PLAIN);
            response.setStatus(Status.SERVER_ERROR_INTERNAL);
            return Filter.STOP;
        }
    }
}
