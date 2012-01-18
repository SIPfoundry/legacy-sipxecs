/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.setting.Group;

public class SpeadDialOnEdit implements DaoEventListener {
    private SpeedDialManager m_speedDialManager;

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof User) {
            m_speedDialManager.deleteSpeedDialsForUser(((User) entity).getId());
        } else if (entity instanceof Group) {
            m_speedDialManager.deleteSpeedDialsForGroup(((Group) entity).getId());
        }
    }

    @Override
    public void onSave(Object entity) {
    }

    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }
}
