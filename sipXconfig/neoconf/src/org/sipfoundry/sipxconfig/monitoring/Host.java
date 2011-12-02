/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.monitoring;

import java.io.Serializable;

public class Host implements Serializable {
    private String m_hostName;

    public Host() {
    }

    public String getHostName() {
        return m_hostName;
    }

    public void setHostName(String hostName) {
        m_hostName = hostName;
    }
}
