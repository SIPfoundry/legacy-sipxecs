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

import org.codehaus.jackson.annotate.JsonIgnore;
import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.region.Region;

@JsonPropertyOrder(alphabetic = true)
public class DnsTarget {
    private int m_percentage;
    private Region m_regionTarget;
    private Location m_locationTarget;
    private BasicType m_basicTarget;
    private Type m_targetType;

    public enum Type {
        REGION, LOCATION, BASIC;
    }

    enum BasicType {
        LOCAL_REGION, ALL_REGIONS, ALL_OTHER_REGIONS;
    }

    public DnsTarget(Region region) {
        m_regionTarget = region;
        m_targetType = Type.REGION;
    }

    public DnsTarget(Location location) {
        m_locationTarget = location;
        m_targetType = Type.LOCATION;
    }

    public DnsTarget(BasicType basicTarget) {
        m_basicTarget = basicTarget;
        m_targetType = Type.BASIC;
    }

    public int getPercentage() {
        return m_percentage;
    }

    public void setPercentage(int percentage) {
        m_percentage = percentage;
    }

    public Type getTargetType() {
        return m_targetType;
    }

    @JsonIgnore
    public Location getLocation() {
        return m_locationTarget;
    }

    @JsonIgnore
    public Region getRegion() {
        return m_regionTarget;
    }

    @JsonIgnore
    public BasicType getBasicTarget() {
        return m_basicTarget;
    }

    public Object getTargetId() {
        switch (m_targetType) {
        case REGION:
            return m_regionTarget.getId().toString();
        case LOCATION:
            return m_locationTarget.getId().toString();
        case BASIC:
        default:
            return m_basicTarget.name();
        }
    }
}
