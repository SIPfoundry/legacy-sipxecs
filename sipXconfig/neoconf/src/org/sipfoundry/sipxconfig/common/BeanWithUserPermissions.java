/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common;

/**
 * Bean With Internal user
 */
public class BeanWithUserPermissions extends BeanWithId {
    private InternalUser m_internalUser;

    public InternalUser getInternalUser() {
        return m_internalUser;
    }

    public void setInternalUser(InternalUser user) {
        m_internalUser = user;
    }

}
