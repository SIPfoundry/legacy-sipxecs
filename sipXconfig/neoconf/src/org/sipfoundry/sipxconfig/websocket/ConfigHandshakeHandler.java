/**
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
package org.sipfoundry.sipxconfig.websocket;

import java.security.Principal;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.springframework.http.server.ServerHttpRequest;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.web.socket.WebSocketHandler;
import org.springframework.web.socket.server.support.DefaultHandshakeHandler;

public class ConfigHandshakeHandler extends DefaultHandshakeHandler {
    private static final Log LOG = LogFactory.getLog(ConfigHandshakeHandler.class);

    @Override
    protected Principal determineUser(ServerHttpRequest request, WebSocketHandler wsHandler,
            Map<String, Object> attributes) {

        final UserDetailsImpl userDetails = (UserDetailsImpl) SecurityContextHolder.
            getContext().getAuthentication().getPrincipal();
        final String authenticatedUser = userDetails.getCanonicalUserName();

        LOG.debug("Wire authenticated user to websocket: " + authenticatedUser);

        return new Principal() {
            @Override
            public String getName() {
                return authenticatedUser;
            }
        };
    }

}
