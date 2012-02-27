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
    static final Collection<Feature> AFFECTED_FEATURES_ON_CHANGE = Collections.singleton((Feature) Alarms.FEATURE);

    private AlarmDefinition m_definition;

    private boolean m_logEnabled = true;

    private String m_groupName = "disabled";

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

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return AFFECTED_FEATURES_ON_CHANGE;
    }
}
