/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common.event;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;

public abstract class UserGroupSaveDeleteListener implements DaoEventListener {

    public void onDelete(Object entity) {
        Group userGroup = getUserGroup(entity);
        if (userGroup != null) {
            onUserGroupDelete(userGroup);
        }
    }

    public void onSave(Object entity) {
        Group userGroup = getUserGroup(entity);
        if (userGroup != null) {
            onUserGroupSave(userGroup);
        }
    }

    private Group getUserGroup(Object entity) {
        if (entity instanceof Group) {
            Group g = (Group) entity;
            if (User.GROUP_RESOURCE_ID.equals(g.getResource())) {
                return g;
            }
        }
        return null;
    }

    protected void onUserGroupDelete(@SuppressWarnings("unused") Group group) {
        // intentionally empty
    }

    protected void onUserGroupSave(@SuppressWarnings("unused") Group group) {
        // intentionally empty
    }
}
