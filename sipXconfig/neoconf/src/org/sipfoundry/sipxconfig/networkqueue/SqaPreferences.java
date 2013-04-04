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
package org.sipfoundry.sipxconfig.networkqueue;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Feature;

public class SqaPreferences extends BeanWithId implements DeployConfigOnEdit {

    private Location m_location;
    private boolean m_proxy = true;
    private boolean m_rls = true;

    public Location getLocation() {
        return m_location;
    }

    public void setLocation(Location location) {
        m_location = location;
    }

    public boolean isProxy() {
        return m_proxy;
    }

    public void setProxy(boolean proxy) {
        m_proxy = proxy;
    }

    public boolean isRls() {
        return m_rls;
    }

    public void setRls(boolean rls) {
        m_rls = rls;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singletonList((Feature) NetworkQueueManager.FEATURE);
    }
}
