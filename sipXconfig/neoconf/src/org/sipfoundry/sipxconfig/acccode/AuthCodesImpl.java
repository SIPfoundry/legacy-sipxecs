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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.AuthorizationCodeRule;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.annotation.Required;

public class AuthCodesImpl implements ReplicableProvider, DialingRuleProvider, FeatureProvider, AuthCodes,
        ProcessProvider {
    private static final String ALIAS_IN_USE = "&error.aliasinuse";
    private AuthCodeManager m_authCodeManager;
    private AddressManager m_addressManager;
    private FeatureManager m_featureManager;
    private BeanWithSettingsDao<AuthCodeSettings> m_settingsDao;
    private AliasManager m_aliasManager;

    @Override
    public List< ? extends DialingRule> getDialingRules(Location location) {
        List<Location> locations = m_featureManager.getLocationsForEnabledFeature(FEATURE);
        if (locations.isEmpty()) {
            return Collections.emptyList();
        }

        Address fsAddress = m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        List<DialingRule> dialingRules = new ArrayList<DialingRule>();
        AuthCodeSettings settings = getSettings();
        String prefix = settings.getAuthCodePrefix();
        if (StringUtils.isEmpty(prefix)) {
            return Collections.emptyList();
        }
        String fsAddressPort = fsAddress.getAddress() + ':' + fsAddress.getPort();
        AuthorizationCodeRule rule = new AuthorizationCodeRule(prefix, fsAddressPort, "auth");
        rule.appendToGenerationRules(dialingRules);
        return dialingRules;
    }

    public AuthCodeSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(AuthCodeSettings settings) {
        if (!m_aliasManager.canObjectUseAlias(settings, settings.getAuthCodePrefix())) {
            throw new UserException(ALIAS_IN_USE, settings.getAuthCodePrefix());
        }

        for (String alias : settings.getAliasesAsSet()) {
            if (!m_aliasManager.canObjectUseAlias(settings, alias)) {
                throw new UserException(ALIAS_IN_USE, alias);
            }
        }

        m_settingsDao.upsert(settings);
    }

    public boolean isEnabled() {
        return m_featureManager.isFeatureEnabled(FEATURE);
    }

    @Override
    public List<Replicable> getReplicables() {
        List<Location> locations = m_featureManager.getLocationsForEnabledFeature(FEATURE);
        if (locations.isEmpty()) {
            return Collections.emptyList();
        }
        List<Replicable> replicables = new ArrayList<Replicable>();
        replicables.addAll(m_authCodeManager.getAuthCodes());
        replicables.add(getSettings());
        return replicables;
    }

    @Required
    public void setAuthCodeManager(AuthCodeManager authCodeManager) {
        m_authCodeManager = authCodeManager;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    @Required
    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Required
    public void setSettingsDao(BeanWithSettingsDao<AuthCodeSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(ProcessDefinition.sipxDefault("sipxacccode",
                ".*\\s-Dprocname=sipxacccode\\s.*")) : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiredOnSameHost(FEATURE, FreeswitchFeature.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }
}
