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

import java.util.Collection;

import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.common.BeanWithId;

@JsonPropertyOrder(alphabetic = true)
public class DnsFailoverGroup extends BeanWithId {
    private Collection<DnsTarget> m_targets;

    public Collection<DnsTarget> getTargets() {
        return m_targets;
    }

    public void setTargets(Collection<DnsTarget> targets) {
        m_targets = targets;
    }

    DnsTarget findTarget(DnsView view, Integer regionId, String address) {
        for (DnsTarget target : m_targets) {
            switch (target.getTargetType()) {
            case BASIC:
                switch (target.getBasicTarget()) {
                case ALL_OTHER_REGIONS:
                    if (!view.isInsideView(regionId)) {
                        return target;
                    }
                    break;
                case ALL_REGIONS:
                    return target;
                case LOCAL_REGION:
                    if (view.isInsideView(regionId)) {
                        return target;
                    }
                    break;
                default:
                    throw new IllegalStateException("unknown basic target " + target.getBasicTarget());
                }
                break;
            case LOCATION:
                if (target.getLocation().getAddress().equals(address)
                        || target.getLocation().getHostname().equals(address)) {
                    return target;
                }
                break;
            case REGION:
                if (target.getRegion().getId().equals(regionId)) {
                    return target;
                }
                break;
            default:
                throw new IllegalStateException("unknown type " + target.getTargetType());
            }
        }
        return null;
    }
}
