/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import javax.sip.address.SipURI;

import org.apache.log4j.Logger;
import org.restlet.Filter;
import org.restlet.data.MediaType;
import org.restlet.data.Protocol;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;

public class BasicOrDigestAuthenticationFilter extends Filter {

    private static Logger logger = Logger.getLogger(BasicOrDigestAuthenticationFilter.class);

    private SipURI sipUri;

    private Plugin plugin;

    public BasicOrDigestAuthenticationFilter(Plugin plugin) throws Exception {
        String proxyDomain = RestServer.getRestServerConfig().getSipxProxyDomain();

        this.sipUri = RestServer.getAddressFactory().createSipURI(null, proxyDomain);
        this.plugin = plugin;
    }

    @Override
    protected int beforeHandle(Request request, Response response) {
        try {
            if (request.getProtocol().equals(Protocol.HTTP)) {
                /*
                 * Request arrived over HTTP - do the digest authentication filter.
                 */
                return new DigestAuthenticationFilter(plugin).beforeHandle(request, response);
            } else {

                /*
                 * Otherwise do the basic authentication filter.
                 */
                return new BasicAuthenticationFilter(plugin).beforeHandle(request, response);
            }

        } catch (Exception ex) {
            logger.error("Exception in processing request", ex);
            response.setEntity("Processing Error " + ex.getMessage(), MediaType.TEXT_PLAIN);
            response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);
            return Filter.STOP;
        }
    }

}
