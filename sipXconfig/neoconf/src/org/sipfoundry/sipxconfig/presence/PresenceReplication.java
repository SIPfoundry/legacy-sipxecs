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
package org.sipfoundry.sipxconfig.presence;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public class PresenceReplication implements Replicable, ReplicableProvider {
    private static final String ALIAS_RELATION = "acd";
    private PresenceServer m_presenceServer;
    private FeatureManager m_featureManager;

    @Override
    public String getName() {
        return null;
    }

    @Override
    public void setName(String name) {
    }

    @Override
    public Set<DataSet> getDataSets() {
        return Collections.singleton(DataSet.ALIAS);
    }

    @Override
    public String getIdentity(String domainName) {
        return null;
    }

    @Override
    public boolean isValidUser() {
        return true;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        return Collections.emptyMap();
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        Collection<AliasMapping> mappings = new ArrayList<AliasMapping>();
        PresenceSettings settings = m_presenceServer.getSettings();
        if (settings != null) {
            Collection<Location> locations = m_featureManager.getLocationsForEnabledFeature(PresenceServer.FEATURE);
            Location location = locations.iterator().next();
            int presencePort = settings.getApiPort();
            String signInCode = settings.getSignInCode();
            String signOutCode = settings.getSignOutCode();
            mappings.add(createPresenceAliasMapping(signInCode.trim(), presencePort, location.getFqdn()));
            mappings.add(createPresenceAliasMapping(signOutCode.trim(), presencePort, location.getFqdn()));
        }
        return mappings;
    }

    private AliasMapping createPresenceAliasMapping(String code, int port, String fqdn) {
        AliasMapping mapping = new AliasMapping(code, SipUri.format(code, fqdn, port), ALIAS_RELATION);
        return mapping;
    }

    @Override
    public List<Replicable> getReplicables() {
        if (!m_featureManager.isFeatureEnabled(PresenceServer.FEATURE)) {
            return Collections.EMPTY_LIST;
        }
        return Collections.singletonList((Replicable) this);
    }

    public void setPresenceServer(PresenceServer presenceServer) {
        m_presenceServer = presenceServer;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }
}
