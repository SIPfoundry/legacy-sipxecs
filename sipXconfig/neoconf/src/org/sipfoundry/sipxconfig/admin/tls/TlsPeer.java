/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.tls;

import org.sipfoundry.sipxconfig.common.BeanWithUserPermissions;

public class TlsPeer extends BeanWithUserPermissions {
    private String m_name;

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

}
