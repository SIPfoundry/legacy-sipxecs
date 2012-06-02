/*
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.backup;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.feature.Feature;

/**
 * Capture a plan to backup various parts of the system to a backup destination
 */
public class BackupPlan extends BeanWithId implements DeployConfigOnEdit {
    private Integer m_limitedCount = 50;
    private BackupType m_type = BackupType.local;
    private Collection<DailyBackupSchedule> m_schedules = new ArrayList<DailyBackupSchedule>(0);
    private String m_encodedDefinitionString;

    public BackupPlan() {
    }

    public BackupPlan(BackupType type) {
        m_type = type;
    }

    public void addSchedule(DailyBackupSchedule dailySchedule) {
        m_schedules.add(dailySchedule);
        dailySchedule.setBackupPlan(this);
    }

    public Collection<DailyBackupSchedule> getSchedules() {
        return m_schedules;
    }

    public void setSchedules(Collection<DailyBackupSchedule> schedules) {
        m_schedules = schedules;
    }

    public Integer getLimitedCount() {
        return m_limitedCount;
    }

    public void setLimitedCount(Integer limitedCount) {
        m_limitedCount = limitedCount;
    }

    public BackupType getType() {
        return m_type;
    }

    public void setType(BackupType type) {
        m_type = type;
    }

    public Collection<String> getDefinitionIds() {
        return new ArrayList<String>(Arrays.asList(StringUtils.split(m_encodedDefinitionString, ',')));
    }

    public void setDefinitions(Collection<String> definitionIds) {
        m_encodedDefinitionString = StringUtils.join(definitionIds, ',');
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) BackupManager.FEATURE);
    }

    /**
     * Only used for hibernate storage. EnumType comes in hibernate 3.7, we're using 3.5 atm
     */
    public String getEncodedType() {
        return m_type.toString();
    }

    /**
     * Only used for hibernate storage
     */
    public void setEncodedType(String s) {
        m_type = BackupType.valueOf(s);
    }

    /**
     * Only used for hibernate storage
     */
    public String getEncodedDefinitionString() {
        return m_encodedDefinitionString;
    }

    /**
     * Only used for hibernate storage
     */
    public void setEncodedDefinitionString(String encodedDefinitionString) {
        m_encodedDefinitionString = encodedDefinitionString;
    }
}
