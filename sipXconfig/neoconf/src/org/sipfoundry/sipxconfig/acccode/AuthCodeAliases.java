/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
    private AuthCodes m_authCodes;
    private FeatureManager m_featureManager;

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        if (!m_authCodes.isEnabled()) {
            return Collections.emptyList();
        }

        List<Location> locations = m_featureManager.getLocationsForEnabledFeature(AuthCodes.FEATURE);
        Collection ids = Collections.emptyList();
        for (Location location : locations) {
            AuthCodeSettings settings = m_authCodes.getSettings(location);
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
        }
        return ids;
    }

    @Override
    public boolean isAliasInUse(String alias) {
        List<Location> locations = m_featureManager.getLocationsForEnabledFeature(AuthCodes.FEATURE);
        for (Location location : locations) {
            AuthCodeSettings settings = m_authCodes.getSettings(location);
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

    public void setAuthCodes(AuthCodes authCodes) {
        m_authCodes = authCodes;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
