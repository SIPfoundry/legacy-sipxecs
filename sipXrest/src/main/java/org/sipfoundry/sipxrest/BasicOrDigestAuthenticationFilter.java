/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import java.net.InetAddress;
import java.net.UnknownHostException;
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

public class BasicOrDigestAuthenticationFilter extends Filter {

    private static Logger logger = Logger.getLogger(BasicAuthenticationFilter.class);

    private SipURI sipUri;

    private Plugin plugin;

    public BasicOrDigestAuthenticationFilter(Plugin plugin) throws Exception {
        String proxyDomain = RestServer.getRestServerConfig().getSipxProxyDomain();

        this.sipUri = RestServer.getAddressFactory().createSipURI(null, proxyDomain);
        this.plugin = plugin;
    }

    private boolean requestFromSipXDomain(Request request) throws UnknownHostException {
        Collection<Hop> hops = new FindSipServer(logger).getSipxProxyAddresses(sipUri);
        String remoteAddr = request.getClientInfo().getAddress();

        for (Hop hop : hops) {
            if (InetAddress.getByName(hop.getHost()).getHostAddress().equals(remoteAddr)) {
                return true;
            }
        }
        return false;
    }

    @Override
    protected int beforeHandle(Request request, Response response) {
        String remoteAddr = request.getClientInfo().getAddress();
        String scheme = request.getProtocol().getSchemeName();
        try {

            logger.debug("Authentication request " + remoteAddr);
            if (requestFromSipXDomain(request)) {
                if (request.getProtocol().equals(Protocol.HTTPS)) {
                    logger.debug("Request was recieved over HTTPS protocol from sipx domain");
                    return Filter.CONTINUE;
                } else if (request.getProtocol().equals(Protocol.HTTP)) {
                    logger.debug("Request was received over HTTP from sipx domain");
                    response.setEntity("HTTP request from sipx domain - rejecting", MediaType.TEXT_PLAIN);                 
                    response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);
                    return Filter.STOP;
                }
            }

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
