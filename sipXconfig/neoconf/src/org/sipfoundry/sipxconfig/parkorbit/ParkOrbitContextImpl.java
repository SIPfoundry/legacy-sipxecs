/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.parkorbit;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.ExtensionInUseException;
import org.sipfoundry.sipxconfig.common.NameInUseException;
import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;

public class ParkOrbitContextImpl extends SipxHibernateDaoSupport implements ParkOrbitContext, BeanFactoryAware,
        FeatureProvider, AddressProvider, ProcessProvider, FirewallProvider {
    private static final String VALUE = "value";
    private static final String QUERY_PARK_ORBIT_IDS_WITH_ALIAS = "parkOrbitIdsWithAlias";
    private AliasManager m_aliasManager;
    private BeanFactory m_beanFactory;
    private SettingDao m_settingDao;
    private BeanWithSettingsDao<ParkSettings> m_beanWithSettingsDao;

    public ParkSettings getSettings() {
        return m_beanWithSettingsDao.findOrCreateOne();
    }

    public void saveSettings(ParkSettings settings) {
        m_beanWithSettingsDao.upsert(settings);
    }

    public void storeParkOrbit(ParkOrbit parkOrbit) {
        // Check for duplicate names and extensions before saving the park orbit
        String name = parkOrbit.getName();
        String extension = parkOrbit.getExtension();
        final String parkOrbitTypeName = "&label.callPark";
        if (!m_aliasManager.canObjectUseAlias(parkOrbit, name)) {
            throw new NameInUseException(parkOrbitTypeName, name);
        }
        if (!m_aliasManager.canObjectUseAlias(parkOrbit, extension)) {
            throw new ExtensionInUseException(parkOrbitTypeName, extension);
        }

        getDaoEventPublisher().publishSave(parkOrbit);
        getHibernateTemplate().saveOrUpdate(parkOrbit);
    }

    public void removeParkOrbits(Collection ids) {
        if (ids.isEmpty()) {
            return;
        }
        removeAll(ParkOrbit.class, ids);
    }

    public ParkOrbit loadParkOrbit(Integer id) {
        return (ParkOrbit) getHibernateTemplate().load(ParkOrbit.class, id);
    }

    public Collection getParkOrbits() {
        return getHibernateTemplate().loadAll(ParkOrbit.class);
    }

    public String getDefaultMusicOnHold() {
        return getBackgroundMusic().getMusic();
    }

    public void setDefaultMusicOnHold(String music) {
        BackgroundMusic backgroundMusic = getBackgroundMusic();
        backgroundMusic.setMusic(music);
        getHibernateTemplate().saveOrUpdate(backgroundMusic);
    }

    private BackgroundMusic getBackgroundMusic() {
        List musicList = getHibernateTemplate().loadAll(BackgroundMusic.class);
        if (!musicList.isEmpty()) {
            return (BackgroundMusic) musicList.get(0);
        }
        return new BackgroundMusic();
    }

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    public boolean isAliasInUse(String alias) {
        // Look for the ID of a park orbit with the specified alias as its name or extension.
        // If there is one, then the alias is in use.
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam(QUERY_PARK_ORBIT_IDS_WITH_ALIAS, VALUE,
                alias);
        return SipxCollectionUtils.safeSize(objs) > 0;
    }

    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection ids = getHibernateTemplate().findByNamedQueryAndNamedParam(QUERY_PARK_ORBIT_IDS_WITH_ALIAS,
                VALUE, alias);
        Collection bids = BeanId.createBeanIdCollection(ids, ParkOrbit.class);
        return bids;
    }

    /**
     * Remove all park orbits - mostly used for testing
     */
    public void clear() {
        removeAll(ParkOrbit.class);
    }

    public ParkOrbit newParkOrbit() {
        ParkOrbit orbit = (ParkOrbit) m_beanFactory.getBean(ParkOrbit.class.getName(), ParkOrbit.class);

        // All auto attendants share same group: default
        Set groups = orbit.getGroups();
        if (groups == null || groups.isEmpty()) {
            orbit.addGroup(getDefaultParkOrbitGroup());
        }

        return orbit;
    }

    public Group getDefaultParkOrbitGroup() {
        return m_settingDao.getGroupCreateIfNotFound(PARK_ORBIT_GROUP_ID, "default");
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public static boolean isParkOrbitGroup(Group group) {
        return PARK_ORBIT_GROUP_ID.equals(group.getResource());
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        List<DefaultFirewallRule> rules = DefaultFirewallRule.rules(Arrays.asList(SIP_TCP_PORT, SIP_UDP_PORT));
        rules.add(new DefaultFirewallRule(SIP_RTP_PORT, FirewallRule.SystemId.PUBLIC));
        return rules;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!type.equalsAnyOf(SIP_TCP_PORT, SIP_UDP_PORT, SIP_RTP_PORT)) {
            return null;
        }

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        if (locations.isEmpty()) {
            return null;
        }

        ParkSettings settings = getSettings();
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = null;
            if (type.equals(SIP_TCP_PORT)) {
                address = new Address(SIP_TCP_PORT, location.getAddress(), settings.getSipTcpPort());
            } else if (type.equals(SIP_UDP_PORT)) {
                address = new Address(SIP_UDP_PORT, location.getAddress(), settings.getSipUdpPort());
            } else if (type.equals(SIP_RTP_PORT)) {
                int startPort = settings.getRtpPort();
                int maxCalls = settings.getMaxSessions();
                address = new Address(SIP_RTP_PORT, location.getAddress(), startPort);
                // end port is 2x max calls was based on code found in sipXpark/src/main.cpp
                address.setEndPort(startPort + (2 * maxCalls));
            }
            addresses.add(address);
        }
        return addresses;
    }

    public void setBeanWithSettingsDao(BeanWithSettingsDao<ParkSettings> beanWithSettingsDao) {
        m_beanWithSettingsDao = beanWithSettingsDao;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(ProcessDefinition.sipxDefault("sipxpark")) : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiredOnSameHost(FEATURE, ProxyManager.FEATURE);
        validator.singleLocationOnly(FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }
}
