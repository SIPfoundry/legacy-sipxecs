/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.security;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletResponse;

import org.acegisecurity.AuthenticationException;
import org.acegisecurity.ui.digestauth.DigestProcessingFilter;
import org.acegisecurity.ui.digestauth.DigestProcessingFilterEntryPoint;
import org.acegisecurity.ui.digestauth.NonceExpiredException;
import org.apache.commons.codec.binary.Base64;
import org.apache.commons.codec.digest.DigestUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

/**
 * Overwrites default implementation of digest filter entry point to allow for injecting realm
 * from sipx domain manager
 */
public class SipxDigestProcessingFilterEntryPoint extends DigestProcessingFilterEntryPoint {
    private static final String SEP = ":";

    private static final Log LOG = LogFactory.getLog(DigestProcessingFilter.class);

    private DomainManager m_domainManager;

    public SipxDigestProcessingFilterEntryPoint() {
        // need to initialize realm to something because super checks for it
        setRealmName("unused");
    }

    @Override
    public void commence(ServletRequest request, ServletResponse response, AuthenticationException authException)
        throws IOException, ServletException {
        HttpServletResponse httpResponse = (HttpServletResponse) response;

        // compute a nonce (do not use remote IP address due to proxy farms)
        // format of nonce is:
        // base64(expirationTime + ":" + md5Hex(expirationTime + ":" + key))
        long expiryTime = System.currentTimeMillis() + (getNonceValiditySeconds() * 1000);
        String signatureValue = new String(DigestUtils.md5Hex(expiryTime + SEP + getKey()));
        String nonceValue = expiryTime + SEP + signatureValue;
        String nonceValueBase64 = new String(Base64.encodeBase64(nonceValue.getBytes()));

        // qop is quality of protection, as defined by RFC 2617.
        // we do not use opaque due to IE violation of RFC 2617 in not
        // representing opaque on subsequent requests in same session.
        String authenticateHeader = String.format("Digest realm=\"%s\", qop=\"auth\", nonce=\"%s\"", getRealmName(),
                nonceValueBase64);

        if (authException instanceof NonceExpiredException) {
            authenticateHeader = authenticateHeader + ", stale=\"true\"";
        }

        if (LOG.isDebugEnabled()) {
            LOG.debug("WWW-Authenticate header sent to user agent: " + authenticateHeader);
        }

        httpResponse.addHeader("WWW-Authenticate", authenticateHeader);
        httpResponse.sendError(HttpServletResponse.SC_UNAUTHORIZED, authException.getMessage());
    }

    @Override
    public String getRealmName() {
        return m_domainManager.getAuthorizationRealm();
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
