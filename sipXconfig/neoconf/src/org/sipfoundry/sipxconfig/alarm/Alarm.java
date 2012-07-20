/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.alarm;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.feature.Feature;

public class Alarm extends BeanWithId implements DeployConfigOnEdit {
    private static final Collection<Feature> AFFECTED_FEATURES_ON_CHANGE = Collections
            .singleton((Feature) Alarms.FEATURE);
    private static final String DISABLED_GROUP_NAME = "disabled";

    private AlarmDefinition m_definition;

    private boolean m_logEnabled = true;

    private String m_groupName = DISABLED_GROUP_NAME;

    private int m_minThreshold;

    public Alarm(AlarmDefinition def) {
        m_definition = def;
        m_minThreshold = def.getDefaultMinimumThreshold();
    }

    public AlarmDefinition getAlarmDefinition() {
        return m_definition;
    }

    public String getGroupName() {
        return m_groupName;
    }

    public void setGroupName(String groupName) {
        m_groupName = groupName;
    }

    public boolean isLogEnabled() {
        return m_logEnabled;
    }

    public void setLogEnabled(boolean logEnabled) {
        m_logEnabled = logEnabled;
    }

    public int getMinThreshold() {
        return m_minThreshold;
    }

    public void setMinThreshold(int minThreshold) {
        m_minThreshold = minThreshold;
    }

    public boolean isEnabled() {
        return !DISABLED_GROUP_NAME.equals(m_groupName);
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return AFFECTED_FEATURES_ON_CHANGE;
    }
}
