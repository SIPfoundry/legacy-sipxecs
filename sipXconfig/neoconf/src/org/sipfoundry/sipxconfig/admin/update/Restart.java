/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin.update;

import java.io.Serializable;

import org.sipfoundry.sipxconfig.admin.WaitingListener;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class Restart implements Serializable, WaitingListener {

    private final UpdateApi m_updateApi;

    private final Location m_location;

    public Restart(UpdateApi updateApi, Location location) {
        m_updateApi = updateApi;
        m_location = location;
    }

    public void afterResponseSent() {
        m_updateApi.restart(m_location);
    }
}
