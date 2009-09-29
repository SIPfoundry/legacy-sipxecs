/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.presence;

import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.common.User;

public interface PresenceServer {

    public void signIn(User user, AcdServer acdServer);

    public void signOut(User user, AcdServer acdServer);

    public PresenceStatus getStatus(User user, AcdServer acdServer);
}
