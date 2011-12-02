/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.freeswitch;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationBeanManager;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.conference.FreeswitchApi;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.service.LoggingEntity;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService.SystemMohSetting;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

public class FreeswitchFeature implements LoggingEntity, Replicable, ReplicableProvider, FeatureProvider, AliasOwner {
    public static final LocationFeature FEATURE = new LocationFeature("freeSwitch");
    public static final AddressType SIP_ADDRESS = new AddressType("freeswitch-sip");
    public static final AddressType XMLRPC_ADDRESS = new AddressType("freeswitch-xmlrpc");
    
    
    private static final String ALIAS_RELATION = "moh";
    
    private FeatureManager m_featureManager;
    private MusicOnHoldManager m_musicOnHoldManager;
    private ApiProvider<FreeswitchApi> m_freeswitchApiProvider;
    private LocationBeanManager m_locationBeanManager;

    public boolean isCodecG729Installed() {        
        List<Location> locations = m_featureManager.getLocationsForEnabledFeature(FreeswitchFeature.FS);
        String serviceUri = null;
        FreeswitchApi api = null;
        String result = null;
        for (Location location : locations) {
            serviceUri = getServiceUri(location);
            api = m_freeswitchApiProvider.getApi(serviceUri);
            try {
                result = api.g729_status();
                if (StringUtils.contains(result, G729_STATUS)) {
                    return true;
                }
                // try also new FS detection algorithm
                result = api.g729_available();
                if (StringUtils.contains(result, "true")) {
                    return true;
                }
            } catch (XmlRpcRemoteException xrre) {
                LOG.error(xrre);
                return false;
            }
        }
        return false;
    }
    
    String getServiceUri(Location l) {
        FreeswitchSettings settings = m_locationBeanManager.getBean(FreeswitchSettings.class, l);
        Address address = getXmlRpcAddress(l);
        return format("http://%s:%d/RPC2", address.getAddress(), address.getPort());        
    }
    
    Address getXmlRpcAddress(Location l) {
        return new Address(l.getAddress(), PORT)
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Required
    public void setFreeswitchApiProvider(ApiProvider<FreeswitchApi> freeswitchApiProvider) {
        m_freeswitchApiProvider = freeswitchApiProvider;
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
        return Collections.EMPTY_MAP;
    }

    @Override
    public boolean isAliasInUse(String alias) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        // TODO Auto-generated method stub
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
}
