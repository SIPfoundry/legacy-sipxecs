/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.presence;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Hashtable;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressRequester;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class PresenceServer implements FeatureProvider, AddressProvider, BeanFactoryAware {
    public static final LocationFeature FEATURE = new LocationFeature("acdPresence");
    public static final AddressType HTTP_ADDRESS = new AddressType("acdPresenceApi");
    public static final AddressType UDP_SIP_ADDRESS = new AddressType("acdPresenceUdp");
    public static final AddressType TCP_SIP_ADDRESS = new AddressType("acdPresenceTcp");
    public static final AddressType TLS_SIP_ADDRESS = new AddressType("acdPresenceTls");    
    public static final String OBJECT_CLASS_KEY = "object-class";
    private static final Log LOG = LogFactory.getLog(PresenceServer.class);
    private static final List<AddressType> ADDRESSES = Arrays.asList(HTTP_ADDRESS, UDP_SIP_ADDRESS, TCP_SIP_ADDRESS,
            TLS_SIP_ADDRESS);
    private CoreContext m_coreContext;
    private FeatureManager m_featureManager;
    private BeanWithSettingsDao<PresenceSettings> m_settingsDao;
    private ListableBeanFactory m_beanFactory;
    private AddressManager m_addressManager;

    public PresenceSettings getSettings() {
        return m_settingsDao.findOne();
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
        String presenceServiceUri = m_addressManager.getSingleAddress(PresenceServer.API_ADDRESS).toString();
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
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes() {
        return ADDRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressType type, AddressRequester source) {
        if (ADDRESSES.contains(type)) {
            PresenceSettings settings = getSettings();
            if (settings != null) {
                if (type.equals(HTTP_ADDRESS)) {
                    Location location = m_featureManager.getLocationForEnabledFeature(FEATURE);
                    Address address = new Address(location.getAddress(), settings.getApiPort(), "http://%s:%d/RPC2");
                }
                return Collections.singleton(address);
            }
        }
        return null;
    }
}
