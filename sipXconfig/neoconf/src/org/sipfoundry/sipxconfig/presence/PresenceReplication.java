/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
        return Collections.singletonList((Replicable) this);
    }

    public void setPresenceServer(PresenceServer presenceServer) {
        m_presenceServer = presenceServer;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
