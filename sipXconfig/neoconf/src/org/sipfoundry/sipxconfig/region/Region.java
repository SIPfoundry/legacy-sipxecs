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
package org.sipfoundry.sipxconfig.region;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.builder.ToStringBuilder;
import org.apache.commons.lang.builder.ToStringStyle;
import org.codehaus.jackson.annotate.JsonIgnore;
import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

@JsonPropertyOrder(alphabetic = true)
public class Region extends BeanWithId implements NamedObject, DeployConfigOnEdit, SystemAuditable {
    public static final Region DEFAULT = new Region("default");
    private String m_name;
    private String[] m_addresses;

    public Region() {
    }

    public Region(String name) {
        m_name = name;
    }

    public static final Map<Integer, List<Location>> locationsByRegion(Collection<Location> locations) {
        Map<Integer, List<Location>> map = new HashMap<Integer, List<Location>>();
        for (Location location : locations) {
            if (location.getRegionId() != null) {
                List<Location> list = map.get(location.getRegionId());
                if (list == null) {
                    list = new ArrayList<Location>();
                    map.put(location.getRegionId(), list);
                }
                list.add(location);
            }
        }

        return map;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    @Override
    @JsonIgnore
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) RegionManager.FEATURE_ID);
    }

    public String toString() {
        String s = ToStringBuilder.reflectionToString(this, ToStringStyle.SHORT_PREFIX_STYLE);
        return s;
    }

    public String[] getAddresses() {
        return m_addresses;
    }

    public void setAddresses(String[] addresses) {
        m_addresses = addresses;
    }

    @Override
    public String getEntityIdentifier() {
        return getName();
    }

    @Override
    public ConfigChangeType getConfigChangeType() {
        return ConfigChangeType.REGION;
    }
}
