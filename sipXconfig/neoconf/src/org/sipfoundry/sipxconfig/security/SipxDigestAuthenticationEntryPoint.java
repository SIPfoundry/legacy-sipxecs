/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.security;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.codec.digest.DigestUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.core.AuthenticationException;
import org.springframework.security.web.authentication.www.DigestAuthenticationEntryPoint;
import org.springframework.security.web.authentication.www.NonceExpiredException;

/**
 * Overwrites default implementation of digest filter entry point to allow for injecting realm
 * from sipx domain manager
 */
public class SipxDigestAuthenticationEntryPoint extends DigestAuthenticationEntryPoint {
    private static final String SEP = ":";

    private static final Log LOG = LogFactory.getLog(SipxDigestAuthenticationEntryPoint.class);

    private DomainManager m_domainManager;

    public SipxDigestAuthenticationEntryPoint() {
        // need to initialize realm to something because super checks for it
        setRealmName("unused");
    }

    @Override
    public void commence(HttpServletRequest request, HttpServletResponse httpResponse,
            AuthenticationException authException) throws IOException, ServletException {
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
