/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.login;

import org.sipfoundry.sipxconfig.common.User;

public interface LoginContext {

    User checkCredentials(String userName, String password);

    String getEncodedPassword(String userName, String password);

    boolean isAdmin(Integer userId);

    LoginEvent[] getUserLoginLog(LogFilter filter);
}
