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

import java.util.ArrayList;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;

/**
 * Collection of speeddial buttons associated with the group.
 */
public class SpeedDialGroup  extends SpeedDialButtons {
    private Group m_userGroup;

    public Group getUserGroup() {
        return m_userGroup;
    }

    public void setUserGroup(Group userGroup) {
        m_userGroup = userGroup;
    }

    public SpeedDial getSpeedDial(User user) {
        SpeedDial userSpeedDial = new SpeedDial();
        userSpeedDial.setUser(user);
        userSpeedDial.setButtons(new ArrayList<Button>(getButtons()));
        return userSpeedDial;
    }
}
