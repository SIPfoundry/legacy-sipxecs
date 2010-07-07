/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;

/**
 * Collection of speeddial buttons associated with the user.
 */
public class SpeedDial extends SpeedDialButtons {
    private User m_user;

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    /**
     * Returns the URL of the resource list corresponding to this speed dial.
     *
     * @param consolidated needs to be set to true for phones that are not RFC compliant and
     *        expect "consolidated" view of resource list (see XCF-1453)
     */
    public String getResourceListId(boolean consolidated) {
        return getResourceListId(getUser().getUserName(), consolidated);
    }

    public static String getResourceListId(String name, boolean consolidated) {
        StringBuilder listId = new StringBuilder("~~rl~");
        listId.append(consolidated ? 'C' : 'F');
        listId.append('~');
        listId.append(name);
        return listId.toString();
    }

    public String getResourceListName() {
        return getUser().getUserName();
    }

    /**
     * If at least one button supports BLF we need to register this list as BLF
     */
    public boolean isBlf() {
        // check if the user has the "Subscribe to Presence" permission since the blf flag
        // might not be set to FALSE even when the user doesn't have this permission(this could happen
        // when the user use the group's speed dial)
        if (m_user != null && !m_user.hasPermission(PermissionName.SUBSCRIBE_TO_PRESENCE)) {
            return false;
        }

        for (Button button : getButtons()) {
            if (button.isBlf()) {
                return true;
            }
        }
        return false;
    }
}
