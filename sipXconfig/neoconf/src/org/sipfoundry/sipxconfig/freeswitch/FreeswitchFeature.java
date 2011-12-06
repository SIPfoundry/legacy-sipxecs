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
import org.sipfoundry.sipxconfig.address.AddressRequester;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationBeanManager;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService.SystemMohSetting;
import org.sipfoundry.sipxconfig.setting.BeanWithLocationDao;

public class FreeswitchFeature implements Replicable, ReplicableProvider, FeatureProvider,
        AliasOwner, AddressProvider {
    public static final LocationFeature FEATURE = new LocationFeature("freeSwitch");
    public static final AddressType SIP_ADDRESS = new AddressType("freeswitch-sip");
    public static final AddressType XMLRPC_ADDRESS = new AddressType("freeswitch-xmlrpc");
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(SIP_ADDRESS, XMLRPC_ADDRESS);
    private static final String ALIAS_RELATION = "moh";

    private FeatureManager m_featureManager;
    private MusicOnHoldManager m_musicOnHoldManager;
    private LocationBeanManager m_locationBeanManager;
    private BeanWithLocationDao<FreeswitchSettings> m_settingsDao;
    private String m_name = "freeswitch";

    public FreeswitchSettings getSettings(Location location) {
        return m_settingsDao.findOne(location);
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
        Collection<Location> locations = m_featureManager.getLocationsForEnabledFeature(FEATURE);
        List<AliasMapping> aliasMappings = new ArrayList<AliasMapping>(1);
        for (Location location : locations) {
            FreeswitchSettings setttings = m_locationBeanManager.getBean(FreeswitchSettings.class, location);
            String mohSetting = setttings.getMusicOnHoldSource();
            String contact = null;
            switch (SystemMohSetting.parseSetting(mohSetting)) {
            case SOUNDCARD_SRC:
                contact = m_musicOnHoldManager.getPortAudioMohUriMapping();
                break;
            case NONE:
                contact = m_musicOnHoldManager.getNoneMohUriMapping();
                break;
            case FILES_SRC:
            default:
                contact = m_musicOnHoldManager.getLocalFilesMohUriMapping();
                break;
            }

            aliasMappings.add(new AliasMapping(m_musicOnHoldManager.getDefaultMohUri(), contact, ALIAS_RELATION));
            aliasMappings.add(new AliasMapping(m_musicOnHoldManager.getLocalFilesMohUri(), m_musicOnHoldManager
                    .getLocalFilesMohUriMapping(), ALIAS_RELATION));
            aliasMappings.add(new AliasMapping(m_musicOnHoldManager.getPortAudioMohUri(), m_musicOnHoldManager
                    .getPortAudioMohUriMapping(), ALIAS_RELATION));
            aliasMappings.add(new AliasMapping(m_musicOnHoldManager.getNoneMohUri(), m_musicOnHoldManager
                    .getNoneMohUriMapping(), ALIAS_RELATION));

            return aliasMappings;
        }

        return aliasMappings;
    }

    public void setLocationBeanManager(LocationBeanManager locationBeanManager) {
        m_locationBeanManager = locationBeanManager;
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return Collections.singleton(XMLRPC_ADDRESS);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            AddressRequester requester) {
        if (ADDRESSES.contains(type)) {
            List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
            List<Address> addresses = new ArrayList<Address>();
            for (Location location : locations) {
                FreeswitchSettings settings = getSettings(location);
                Address address = new Address();
                address.setAddress(location.getAddress());
                if (type.equals(XMLRPC_ADDRESS)) {
                    address.setFormat("http://%s:%d/RPC2");
                    address.setPort(settings.getXmlRpcPort());
                } else {
                    address.setPort(settings.getFreeswitchSipPort());
                }
                addresses.add(address);
            }
        }
        return null;
    }

    @Override
    public String getName() {
        return m_name;
    }

    @Override
    public void setName(String name) {
        m_name = name;
    }
}
