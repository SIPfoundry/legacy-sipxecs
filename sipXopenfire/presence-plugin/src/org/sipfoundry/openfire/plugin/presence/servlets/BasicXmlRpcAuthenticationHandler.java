/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.openfire.plugin.presence.servlets;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.XmlRpcRequest;
import org.apache.xmlrpc.common.XmlRpcHttpRequestConfig;
import org.apache.xmlrpc.server.AbstractReflectiveHandlerMapping.AuthenticationHandler;
import org.sipfoundry.commons.security.Md5Encoder;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.util.DomainConfiguration;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

public class BasicXmlRpcAuthenticationHandler implements AuthenticationHandler {

    private static Logger logger = Logger.getLogger(BasicXmlRpcAuthenticationHandler.class);
    private String m_sharedSecret = null;

    private boolean isAuthenticated(String username, String password) {
        if (m_sharedSecret == null) {
            DomainConfiguration config = new DomainConfiguration(System.getProperty("conf.dir")+"/domain-config");
            m_sharedSecret = config.getSharedSecret();
        }

        if (username == null || password == null) {
            logger.warn("Could not authenticate since no username or password was provided");
            return false;
        }
        if (password.equals(m_sharedSecret)) {
            return true;
        }
        try {
            User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUser(username);
            if (user == null) {
                return false;
            }
            String encodedPassword = Md5Encoder.getEncodedPassword(password);
            return user.getPintoken().equals(encodedPassword);
        } catch (Exception ex) {
            logger.warn("Could not authenticate " + username);
            return false;
        }
    }

    @Override
    public boolean isAuthorized(XmlRpcRequest pRequest) throws XmlRpcException {
        XmlRpcHttpRequestConfig config = (XmlRpcHttpRequestConfig) pRequest.getConfig();
        return isAuthenticated(config.getBasicUserName(), config.getBasicPassword());
    }

}
