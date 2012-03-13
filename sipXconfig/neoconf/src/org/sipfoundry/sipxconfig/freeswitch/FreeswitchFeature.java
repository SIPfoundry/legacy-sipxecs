/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.freeswitch;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.commserver.SettingsWithLocationDao;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings.SystemMohSetting;
import org.sipfoundry.sipxconfig.moh.MohAddressFactory;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class FreeswitchFeature implements Replicable, ReplicableProvider, FeatureProvider, AliasOwner,
        AddressProvider, ProcessProvider {
    public static final LocationFeature FEATURE = new LocationFeature("freeSwitch");
    public static final AddressType SIP_ADDRESS = AddressType.sip("freeswitch-sip");
    public static final AddressType XMLRPC_ADDRESS = new AddressType("freeswitch-xmlrpc", "http://%s:%d/RPC2");
    public static final AddressType EVENT_ADDRESS = new AddressType("freeswitch-event");
    public static final AddressType ACC_EVENT_ADDRESS = new AddressType("acc-freeswitch-event");
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(SIP_ADDRESS, XMLRPC_ADDRESS,
            EVENT_ADDRESS, ACC_EVENT_ADDRESS);
    private static final String ALIAS_RELATION = "moh";

    private FeatureManager m_featureManager;
    private MusicOnHoldManager m_musicOnHoldManager;
    private SettingsWithLocationDao<FreeswitchSettings> m_settingsDao;
    private String m_name = "freeswitch";

    public FreeswitchSettings getSettings(Location location) {
        return m_settingsDao.findOrCreate(location);
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> ds = new HashSet<DataSet>();
        ds.add(DataSet.ALIAS);
        return ds;
    }

    @Override
    public String getIdentity(String domain) {
        return null;
    }

    @Override
    public List<Replicable> getReplicables() {
        List<Replicable> replicables = new ArrayList<Replicable>();
        replicables.add(this);
        return replicables;
    }

    @Override
    public boolean isValidUser() {
        return false;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        return Collections.emptyMap();
    }

    @Override
    public boolean isAliasInUse(String alias) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        return null;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        if (!m_featureManager.isFeatureEnabled(FEATURE)) {
            return null;
        }
        Collection<Location> locations = m_featureManager.getLocationsForEnabledFeature(FEATURE);
        List<AliasMapping> aliasMappings = new ArrayList<AliasMapping>(locations.size() * 4);
        MohAddressFactory moh = m_musicOnHoldManager.getAddressFactory();
        for (Location location : locations) {
            FreeswitchSettings setttings = m_settingsDao.findOrCreate(location);
            String mohSetting = setttings.getMusicOnHoldSource();
            String contact = null;
            switch (SystemMohSetting.parseSetting(mohSetting)) {
            case SOUNDCARD_SRC:
                contact = moh.getPortAudioMohUriMapping();
                break;
            case NONE:
                contact = moh.getNoneMohUriMapping();
                break;
            case FILES_SRC:
            default:
                contact = moh.getLocalFilesMohUriMapping();
                break;
            }

            aliasMappings.add(new AliasMapping(moh.getDefaultMohUri(), contact, ALIAS_RELATION));
            aliasMappings.add(new AliasMapping(moh.getLocalFilesMohUri(), moh.getLocalFilesMohUriMapping(),
                    ALIAS_RELATION));
            aliasMappings.add(new AliasMapping(moh.getPortAudioMohUri(), moh.getPortAudioMohUriMapping(),
                    ALIAS_RELATION));
            aliasMappings.add(new AliasMapping(moh.getNoneMohUri(), moh.getNoneMohUriMapping(), ALIAS_RELATION));

            return aliasMappings;
        }

        return aliasMappings;
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return Collections.singleton(XMLRPC_ADDRESS);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Object requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        if (locations.isEmpty()) {
            return null;
        }

        List<Address> addresses = new ArrayList<Address>();
        for (Location location : locations) {
            FreeswitchSettings settings = getSettings(location);
            Address address = null;
            if (type.equals(XMLRPC_ADDRESS)) {
                address = new Address(XMLRPC_ADDRESS, location.getAddress(), settings.getXmlRpcPort());
            } else if (type.equals(EVENT_ADDRESS)) {
                address = new Address(EVENT_ADDRESS, location.getAddress(), settings.getEventSocketPort());
            } else if (type.equals(ACC_EVENT_ADDRESS)) {
                address = new Address(ACC_EVENT_ADDRESS, location.getAddress(), settings.getAccEventSocketPort());
            } else if (type.equals(SIP_ADDRESS)) {
                address = new Address(SIP_ADDRESS, location.getAddress(), settings.getFreeswitchSipPort());
            }
            addresses.add(address);
        }

        return addresses;
    }

    @Override
    public String getName() {
        return m_name;
    }

    @Override
    public void setName(String name) {
        m_name = name;
    }

    public void setMusicOnHoldManager(MusicOnHoldManager musicOnHoldManager) {
        m_musicOnHoldManager = musicOnHoldManager;
    }

    public void setSettingsDao(SettingsWithLocationDao<FreeswitchSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(new ProcessDefinition(m_name)) : null);
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isBasic()) {
            b.addFeature(FEATURE);
        }
    }
}
