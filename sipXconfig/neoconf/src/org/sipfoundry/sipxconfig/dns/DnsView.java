/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.dns;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.codehaus.jackson.annotate.JsonIgnore;
import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.feature.Feature;

@JsonPropertyOrder(alphabetic = true)
public class DnsView extends BeanWithId implements NamedObject, DeployConfigOnEdit {
    private Integer m_regionId;
    private String m_name;
    private Integer m_planId;
    private ExcludedRecords[] m_excluded;
    private boolean m_enabled = true;
    private List<Integer> m_customRecordsIds;
    public enum ExcludedRecords {
        NS, NAPTR, A
    }

    public DnsView() {
    }

    public DnsView(String name) {
        m_name = name;
    }

    public boolean isInsideView(Integer regionId) {
        if (m_regionId != null) {
            if (m_regionId.equals(regionId)) {
                return true;
            }
        }

        return false;
    }

    @JsonIgnore
    public String getConfigFriendlyName() {
        return m_name.replaceAll("\\W+", "_");
    }

    public void setRegionId(Integer regionId) {
        m_regionId = regionId;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public Integer getRegionId() {
        return m_regionId;
    }

    public Integer getPlanId() {
        return m_planId;
    }

    public void setPlanId(Integer planId) {
        m_planId = planId;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    @Override
    @JsonIgnore
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) DnsManager.FEATURE);
    }

    public List<Integer> getCustomRecordsIds() {
        return m_customRecordsIds;
    }

    public void setCustomRecordsIds(List<Integer> customRecordIds) {
        m_customRecordsIds = customRecordIds;
    }

    public ExcludedRecords[] getExcluded() {
        return m_excluded;
    }

    public void setExcluded(ExcludedRecords[] excluded) {
        m_excluded = excluded;
    }
}
