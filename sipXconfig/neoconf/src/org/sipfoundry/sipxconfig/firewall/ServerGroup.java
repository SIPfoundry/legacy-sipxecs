/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.firewall;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.feature.Feature;

public class ServerGroup extends BeanWithId implements NamedObject, DeployConfigOnEdit {
    private String m_name;
    private String m_serverList;

    public ServerGroup() {
    }

    public ServerGroup(String name, String list) {
        m_name = name;
        m_serverList = list;
    }

    @Override
    public String getName() {
        return m_name;
    }

    @Override
    public void setName(String name) {
        m_name = name;
    }

    public void setServerList(String list) {
        m_serverList = list;
    }

    public String getServerList() {
        return m_serverList;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) FirewallManager.FEATURE);
    }
}
