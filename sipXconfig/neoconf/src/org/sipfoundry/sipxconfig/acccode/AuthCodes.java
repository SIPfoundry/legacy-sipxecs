/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acccode;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.AuthorizationCodeRule;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class AuthCodes implements ReplicableProvider, DialingRuleProvider, FeatureProvider {
    public static final LocationFeature FEATURE = new LocationFeature("authCode");
    private AuthCodeManager m_authCodeManager;
    private AddressManager m_addressManager;
    private FeatureManager m_featureManager;
    private BeanWithSettingsDao<AuthCodeSettings> m_settingsDao;

    public List< ? extends DialingRule> getDialingRules() {
        AuthCodeSettings settings = getSettings();
        if (settings == null) {
            return Collections.emptyList();
        }
        Address fsAddress = m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        List<DialingRule> dialingRules = new ArrayList<DialingRule>();
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

    public boolean isEnabled() {
        return m_featureManager.isFeatureEnabled(FEATURE);
    }

    @Override
    public List<Replicable> getReplicables() {
        List<Replicable> replicables = new ArrayList<Replicable>();
        replicables.addAll(m_authCodeManager.getAuthCodes());
        replicables.add(getSettings());
        return replicables;
    }

    public void setAuthCodeManager(AuthCodeManager authCodeManager) {
        m_authCodeManager = authCodeManager;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setSettingsDao(BeanWithSettingsDao<AuthCodeSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }
}
