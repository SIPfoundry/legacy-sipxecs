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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.region.Region;

/**
 * Restrict deleting regions or locations that have DNS plans attached to them
 */
public class DnsValidation implements DaoEventListener {
    private DnsManager m_dnsManager;

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof Region) {
            checkInUse(m_dnsManager.getViewNamesUsingRegion((Region) entity), "&err.regionInUseByView");
            checkInUse(m_dnsManager.getPlanNamesUsingRegion((Region) entity), "&err.regionInUseByPlan");
        }
        if (entity instanceof Location) {
            checkInUse(m_dnsManager.getPlanNamesUsingLocation((Location) entity), "&err.locationInUseByPlan");
        }
        if (entity instanceof DnsFailoverPlan) {
            checkInUse(m_dnsManager.getViewNamesUsingPlan((DnsFailoverPlan) entity), "&err.planInUseByView");
        }
    }

    @Override
    public void onSave(Object entity) {
    }

    void checkInUse(String[] names, String errResourceId) {
        if (names != null && names.length > 0) {
            String namesStr = StringUtils.join(names, ", ");
            throw new UserException(errResourceId, namesStr);
        }
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }
}
