/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import java.net.InetAddress;
import java.util.Collection;
import java.util.Random;

import javax.sip.address.Hop;
import javax.sip.address.SipURI;

import org.apache.log4j.Logger;
import org.restlet.Filter;
import org.restlet.data.ChallengeRequest;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.ChallengeScheme;
import org.restlet.data.MediaType;
import org.restlet.data.Parameter;
import org.restlet.data.Protocol;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.util.Series;
import org.sipfoundry.commons.siprouter.FindSipServer;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;

public class DigestAuthenticationFilter extends Filter {
    private static Logger logger = Logger.getLogger(DigestAuthenticationFilter.class);
    private Plugin plugin;

    private static Random random = new Random();

    /**
     * Defined in rfc 2617 as KD(secret, data) = H(concat(secret, ":", data))
     * 
     * @param data data
     * @param secret secret
     * @return H(concat(secret, ":", data));
     */
    private static String KD(String secret, String data) {
        return Util.H(secret + ":" + data);
    }

  
    public DigestAuthenticationFilter(Plugin plugin) {
        this.plugin = plugin;
    }

   

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

            if (! plugin.getMetaInf().getSecurity().equals(MetaInf.LOCAL_AND_REMOTE)) {
                response.setStatus(Status.CLIENT_ERROR_FORBIDDEN);
                response.setEntity("Remote access is not permitted ", MediaType.TEXT_PLAIN);
                logger.debug("Cannot process request ");
                return Filter.STOP;
            } else {
                String agentName = plugin.getAgent(request);
                logger.debug("AgentName = " + agentName);
                ValidUsersXML validUsers = ValidUsersXML.update(logger, true);

                User user = validUsers.getUser(agentName);

                if (user == null) {
                    logger.debug("User not found");
                    response.setEntity("User Not found " + agentName, MediaType.TEXT_PLAIN);
                    response.setStatus(Status.CLIENT_ERROR_FORBIDDEN);
                    return Filter.STOP;
                }

                if (user.getPintoken() == null) {
                    logger.debug("PINTOKEN is null");
                    return Filter.CONTINUE;
                }
                ChallengeResponse challengeResponse = request.getChallengeResponse();

                logger.debug("challengeResponse is " + challengeResponse);
                if (challengeResponse == null) {
                    logger.debug("Requesting DIGEST credentials");
                    ChallengeRequest challengeRequest = new ChallengeRequest(
                            ChallengeScheme.HTTP_DIGEST, RestServer.getRestServerConfig()
                                    .getSipxProxyDomain());
                    response.setChallengeRequest(challengeRequest);
                    response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);

                    return Filter.STOP;
                }

                Series<Parameter> parameters = challengeResponse.getParameters();
                if (parameters.isEmpty()) {
                    String nonce = Util.H(Long.toString(Math.abs(random.nextLong())));
                    ChallengeRequest challengeRequest = new ChallengeRequest(
                            ChallengeScheme.HTTP_DIGEST, RestServer.getRestServerConfig()
                                    .getSipxProxyDomain());
                    response.setChallengeRequest(challengeRequest);
                    response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);

                    parameters.add("qop", "auth");
                    parameters.add("nonce", nonce);
                    parameters.add("algorithm", "MD5");
                    parameters
                            .add("realm", RestServer.getRestServerConfig().getSipxProxyDomain());
                    challengeResponse.setCredentialComponents(parameters);
                    logger.debug("sending DIGEST challenge");
                    return Filter.STOP;
                }

                if (parameters == null || parameters.isEmpty()) {
                    logger.debug("Requesting DIGEST credentials");
                    ChallengeRequest challengeRequest = new ChallengeRequest(
                            ChallengeScheme.HTTP_DIGEST, RestServer.getRestServerConfig()
                                    .getSipxProxyDomain());
                    response.setChallengeRequest(challengeRequest);
                    response.setStatus(Status.CLIENT_ERROR_PROXY_AUTHENTIFICATION_REQUIRED);
                    return Filter.STOP;
                }

                logger.debug("credentials = " + challengeResponse.getCredentials());

                String nonce = challengeResponse.getParameters().getFirstValue("nonce", true);
                String cnonce = challengeResponse.getParameters().getFirstValue("cnonce", true);
                String uri = challengeResponse.getParameters().getFirstValue("uri", true);
                String nc = challengeResponse.getParameters().getFirstValue("nc", true);
                String qop = challengeResponse.getParameters().getFirstValue("qop", true);
                String response_param = challengeResponse.getParameters().getFirstValue(
                        "response");
                String method = request.getMethod().getName();

                logger.debug(String.format(
                        "nonce %s cnonce %s uri %s nc %s qop %s response %s method %s", nonce,
                        cnonce, uri, nc, qop, response_param, method));
                String A2 = null;
                if (qop == null || qop.trim().length() == 0
                        || qop.trim().equalsIgnoreCase("auth")) {
                    A2 = method + ":" + uri;
                } else {
                    String entity_digest = response.getEntity().getDigest().toString();

                    A2 = method + ":" + uri + ":" + entity_digest;
                }
                String pintoken = user.getPintoken();
                String expectedValue = null;

                if (cnonce != null && qop != null && nc != null
                        && (qop.equalsIgnoreCase("auth") || qop.equalsIgnoreCase("auth-int")))

                {
                    expectedValue = KD(pintoken, nonce + ":" + nc + ":" + cnonce + ":" + qop
                            + ":" + Util.H(A2));

                } else {
                    expectedValue = KD(pintoken, nonce + ":" + Util.H(A2));
                }

                if (expectedValue.equals(response_param)) {
                    logger.debug("Digest authentication succeeded");
                    return Filter.CONTINUE;
                } else {

                    logger.debug("User not authenticated - token mismatch ");
                    response.setEntity("pin mismatch", MediaType.TEXT_PLAIN);
                    response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);
                    return Filter.STOP;
                }
            } 
        } catch (Exception ex) {
            logger.error("Exception in processing request", ex);
            response.setEntity("Processing Error " + ex.getMessage(), MediaType.TEXT_PLAIN);
            response.setStatus(Status.CLIENT_ERROR_UNAUTHORIZED);
            return Filter.STOP;
        }
    }

}
