/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tls;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithUserPermissions;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

public class TlsPeer extends BeanWithUserPermissions implements DeployConfigOnEdit, SystemAuditable {
    private String m_name;

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) TlsPeerManager.FEATURE);
    }

    @Override
    public String getEntityIdentifier() {
        return getName();
    }

    @Override
    public ConfigChangeType getConfigChangeType() {
        return ConfigChangeType.TLS_PEER;
    }
}
