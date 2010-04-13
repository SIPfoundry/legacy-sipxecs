/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.callgroup;

import org.sipfoundry.sipxconfig.common.User;

public class UserRing extends AbstractRing {
    private CallGroup m_callGroup;
    private User m_user;

    public UserRing() {
        // bean only
    }

    protected Object getUserPart() {
        return m_user.getUserName();
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public CallGroup getCallGroup() {
        return m_callGroup;
    }

    public void setCallGroup(CallGroup callGroup) {
        m_callGroup = callGroup;
    }

    /**
     * Checks if this ring is first in the sequence
     *
     * @return true if this is the first ring
     */
    public boolean isFirst() {
        AbstractRing ring = getCallGroup().getRings().get(0);
        return ring == this;
    }
}
