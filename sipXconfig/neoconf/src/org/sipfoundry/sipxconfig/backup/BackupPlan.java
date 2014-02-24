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
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.codehaus.jackson.annotate.JsonIgnore;
import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.feature.Feature;

/**
 * Capture a plan to backup various parts of the system to a backup destination
 */
@JsonPropertyOrder(alphabetic = true)
public class BackupPlan extends BeanWithId implements DeployConfigOnEdit {
    private Integer m_limitedCount = 50;
    private BackupType m_type = BackupType.local;
    private Collection<DailyBackupSchedule> m_schedules = new ArrayList<DailyBackupSchedule>(0);
    private Set<String> m_definitionIds = new HashSet<String>();
    private boolean m_includeDeviceFiles;

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

    /**
     * What backups to perform/restore
     * @return
     */
    public Set<String> getDefinitionIds() {
        return m_definitionIds;
    }

    public void setDefinitionIds(Set<String> ids) {
        m_definitionIds = ids;
    }

    @Override
    @JsonIgnore
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) BackupManager.FEATURE);
    }

    /**
     * Only used for hibernate storage. EnumType comes in hibernate 3.7, we're using 3.5 atm
     */
    @JsonIgnore
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
    @JsonIgnore
    public String getEncodedDefinitionString() {
        return m_definitionIds.isEmpty() ? null : StringUtils.join(m_definitionIds, ',');
    }

    /**
     * Only used for hibernate storage
     */
    public void setEncodedDefinitionString(String encodedDefinitionString) {
        m_definitionIds = new HashSet<String>();
        if (StringUtils.isBlank(encodedDefinitionString)) {
            return;
        }
        String[] split = StringUtils.split(encodedDefinitionString, ',');
        m_definitionIds.addAll(Arrays.asList(split));
    }

    public boolean isIncludeDeviceFiles() {
        return m_includeDeviceFiles;
    }

    public void setIncludeDeviceFiles(boolean includeDeviceFiles) {
        m_includeDeviceFiles = includeDeviceFiles;
    }
}
