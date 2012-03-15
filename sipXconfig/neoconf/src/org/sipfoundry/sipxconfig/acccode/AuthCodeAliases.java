/**
 *
 *
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
package org.sipfoundry.sipxconfig.acccode;

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public class AuthCodeAliases implements AliasOwner {
    private AuthCodesImpl m_authCodes;
    private FeatureManager m_featureManager;

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection ids = Collections.emptyList();
        if (!m_authCodes.isEnabled()) {
            return ids;
        }

        AuthCodeSettings settings = m_authCodes.getSettings();
        if (settings != null) {
            Set<String> aliases = settings.getAliasesAsSet();
            aliases.add(settings.getAuthCodeAliases());
            for (String serviceAlias : aliases) {
                if (serviceAlias.equals(alias)) {
                    ids = BeanId.createBeanIdCollection(Collections.singletonList(settings.getId()),
                            AuthCodeSettings.class);
                    break;
                }
            }
        }
        return ids;
    }

    @Override
    public boolean isAliasInUse(String alias) {
        List<Location> locations = m_featureManager.getLocationsForEnabledFeature(AuthCodes.FEATURE);
        if (!locations.isEmpty()) {
            AuthCodeSettings settings = m_authCodes.getSettings();
            Set<String> aliases = settings.getAliasesAsSet();
            aliases.add(settings.getAuthCodeAliases());
            for (String serviceAlias : aliases) {
                if (serviceAlias.equals(alias)) {
                    return true;
                }
            }
        }

        return false;
    }

    public void setAuthCodes(AuthCodesImpl authCodes) {
        m_authCodes = authCodes;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
