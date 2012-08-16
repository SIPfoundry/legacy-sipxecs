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
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Hashtable;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.acd.Acd;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
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
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * ACD Presence system. Determining when an agent is available.
 */
public class PresenceServerImpl implements FeatureProvider, AddressProvider, BeanFactoryAware, PresenceServer,
    ProcessProvider, FirewallProvider {
    public static final LocationFeature FEATURE = new LocationFeature("acdPresence");
    public static final AddressType HTTP_ADDRESS = new AddressType("acdPresenceApi", "http://%s:%d/RPC2");
    public static final AddressType SIP_TCP_ADDRESS = new AddressType("acdPresenceTcp");
    public static final AddressType SIP_UDP_ADDRESS = new AddressType("acdPresenceUdp");
    public static final String OBJECT_CLASS_KEY = "object-class";
    private static final Log LOG = LogFactory.getLog(PresenceServer.class);
    private CoreContext m_coreContext;
    private FeatureManager m_featureManager;
    private BeanWithSettingsDao<PresenceSettings> m_settingsDao;
    private ListableBeanFactory m_beanFactory;
    private AddressManager m_addressManager;

    @Override
    public PresenceSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(PresenceSettings settings) {
        m_settingsDao.upsert(settings);
    }

    public void initialize() {
        PresenceSettings settings = getSettings();
        if (settings == null) {
            settings = m_beanFactory.getBean(PresenceSettings.class);
            settings.assignAvailableAliases();
            m_settingsDao.upsert(settings);
        }
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void signIn(User user, AcdServer acdServer) {
        signInAction(SignIn.SIGN_IN, user, acdServer);
    }

    public void signOut(User user, AcdServer acdServer) {
        signInAction(SignIn.SIGN_OUT, user, acdServer);
    }

    public PresenceStatus getStatus(User user, AcdServer acdServer) {
        PresenceStatus status = PresenceStatus.NOT_AVAILABLE;
        if (m_featureManager.isFeatureEnabled(FEATURE)) {
            try {
                Hashtable response = signInAction(SignIn.STATUS, user, acdServer);
                String statusId = (String) response.get(SignIn.RESULT_TEXT);
                status = PresenceStatus.resolve(statusId);
            } catch (Exception e) {
                LOG.error(e);
            }
        }
        return status;
    }


    private Hashtable signInAction(String action, User user, AcdServer acdServer) {
        if (!m_featureManager.isFeatureEnabled(PresenceServer.FEATURE)) {
            return null;
        }
        String presenceServiceUri = m_addressManager.getSingleAddress(PresenceServer.HTTP_ADDRESS).toString();
        XmlRpcProxyFactoryBean factory = new XmlRpcProxyFactoryBean();
        factory.setServiceInterface(SignIn.class);
        factory.setServiceUrl(presenceServiceUri);
        factory.afterPropertiesSet();
        SignIn api = (SignIn) factory.getObject();
        return userAction(api, action, user);
    }

    Hashtable userAction(SignIn api, String actionId, User user) {
        try {
            Hashtable action = new Hashtable();
            String domainName = m_coreContext.getDomainName();
            action.put(OBJECT_CLASS_KEY, SignIn.OBJECT_CLASS_ID);
            action.put(actionId, user.getUri(domainName));
            Hashtable response = api.action(action);
            checkErrorCode(response);
            return response;
        } catch (XmlRpcRemoteException e) {
            LOG.error("XML/RPC related problems in presence service.", e);
            throw new SignInException(e.getCause().getMessage());
        }
    }

    static void checkErrorCode(Hashtable response) {
        Integer resultCode = (Integer) response.get(SignIn.RESULT_CODE);
        if (!SignIn.SUCCESS.equals(resultCode)) {
            String rawMessage = (String) response.get(SignIn.RESULT_TEXT);
            String message = StringUtils.defaultString(rawMessage, "Error calling remote api");
            throw new SignInException(message);
        }
    }

    static class SignInException extends UserException {
        SignInException(String msg) {
            super(msg);
        }
    }

    /**
     * Raw API from presence server. Not very useful outside this context
     */
    public static interface SignIn {
        public static final Integer SUCCESS = new Integer(1);
        public static final String RESULT_CODE = "result-code";
        public static final String RESULT_TEXT = "result-text";
        public static final String OBJECT_CLASS_ID = "login";
        public static final String SIGN_IN = "sign-in";
        public static final String STATUS = "sign-in-status";
        public static final String SIGN_OUT = "sign-out";

        public Hashtable action(Hashtable params);
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
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    @Override
    public java.util.Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Location requester) {
        if (!type.equalsAnyOf(HTTP_ADDRESS, SIP_TCP_ADDRESS, SIP_UDP_ADDRESS)) {
            return null;
        }

        PresenceSettings settings = getSettings();
        List<Location> locations = m_featureManager.getLocationsForEnabledFeature(FEATURE);
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = null;
            if (type.equals(HTTP_ADDRESS)) {
                address = new Address(HTTP_ADDRESS, location.getAddress(), settings.getApiPort());
            } else if (type.equals(SIP_TCP_ADDRESS)) {
                address = new Address(SIP_TCP_ADDRESS, location.getAddress(), settings.getSipTcpPort());
            } else if (type.equals(SIP_UDP_ADDRESS)) {
                address = new Address(SIP_UDP_ADDRESS, location.getAddress(), settings.getSipUdpPort());
            }
            addresses.add(address);
        }

        return addresses;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setSettingsDao(BeanWithSettingsDao<PresenceSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(ProcessDefinition.sipx("sipxpresence")) : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.EXPERIMENTAL) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(Arrays.asList(HTTP_ADDRESS, SIP_TCP_ADDRESS, SIP_UDP_ADDRESS));
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiresAtLeastOne(FEATURE, Acd.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        if (request.getAllNewlyEnabledFeatures().contains(PresenceServer.FEATURE)) {
            initialize();
        }
    }
}
