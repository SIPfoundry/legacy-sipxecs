/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common.event;

import org.sipfoundry.sipxconfig.setting.Group;

public abstract class UserGroupDeleteListener implements DaoEventListener {

    public void onDelete(Object entity) {
        if (entity instanceof Group) {
            onUserGroupDelete((Group) entity);
        }
    }

    public void onSave(Object entity_) {
    }

    protected abstract void onUserGroupDelete(Group user);
}
