/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
