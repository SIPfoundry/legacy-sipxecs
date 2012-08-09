/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.voicemail;

import java.security.Principal;

import javax.servlet.http.HttpServletRequest;

public class ServletUtil {
    public static boolean isForbidden(HttpServletRequest request, String userName, int requestPort, int port) {
        if (requestPort != port) {
            Principal principal = request.getUserPrincipal();
            String authenticatedUserName = (principal == null) ? null : principal.getName();

            return authenticatedUserName != null && (!authenticatedUserName.equals(userName) && !authenticatedUserName.equals("superadmin"));
        } else {
            String trustedUserName = request.getHeader("sipx-user");
            if (trustedUserName != null && !trustedUserName.equals(userName)) {
                return true;
            }
            return false;
        }
    }
}
