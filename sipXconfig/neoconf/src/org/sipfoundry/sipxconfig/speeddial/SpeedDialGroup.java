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
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.rls.Rls;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

/**
 * Collection of speeddial buttons associated with the group.
 */
public class SpeedDialGroup  extends SpeedDialButtons implements DeployConfigOnEdit, SystemAuditable {
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

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) Rls.FEATURE);
    }

    @Override
    public String getEntityIdentifier() {
        return m_userGroup.getName();
    }

    @Override
    public ConfigChangeType getConfigChangeType() {
        return ConfigChangeType.SPEED_DIAL_GROUP;
    }
}
