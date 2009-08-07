/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.openfire;

public interface SipxOpenfireContext {

    boolean saveOpenfireAccount(String previousUserName, String userName, String password, String sipUserName,
            String sipPassword, String email);

    boolean deleteOpenfireAccount(String userName);
}
