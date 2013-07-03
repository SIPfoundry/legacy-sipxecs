/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import java.util.Map;

public interface OpenfireApi {
    Map<String, String> userExists(String userName);

    Map<String, String> createUserAccount(String xmppUserName, String password, String displayName,
            String email);

    Map<String, String> destroyUserAccount(String xmppUserName);

    Map<String, String> setSipId(String xmppUserName, String sipId);

    Map<String, String> setSipPassword(String xmppUserName, String sipPassword);
}
