/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.address;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;

public class AddressManager implements BeanFactoryAware {
    public static final AddressType NTP_ADDRESS = new AddressType("ntp");
    private List<AddressProvider> m_providers;
    private ListableBeanFactory m_beanFactory;
    private FeatureManager m_featureManager;

    public Address getSingleAddress(AddressType type) {
        return getSingleAddress(type, null);
    }

    public Address getSingleAddress(AddressType type, Object requester) {
        // TODO: Consult DNS provider to return SRV addresses when appropriate to return
        // a single address that is really a combination of addresses
        for (AddressProvider provider : getProviders()) {
            Collection<Address> addresses = provider.getAvailableAddresses(this, type, requester);
            if (addresses != null && addresses.size() > 0) {
                return addresses.iterator().next();
            }
        }

        return null;
    }

    public Collection<Address> getAddresses(AddressType type) {
        return getAddresses(type, null);
    }

    public Collection<Address> getAddresses(AddressType type, Object requester) {
        // TODO: Consult DNS provider to return SRV addresses when appropriate to return
        // a single address that is really a combination of addresses
        List<Address> addresses = new ArrayList<Address>();
        for (AddressProvider provider : getProviders()) {
            Collection<Address> some = provider.getAvailableAddresses(this, type, requester);
            if (some != null && !some.isEmpty()) {
                addresses.addAll(some);
            }
        }

        return addresses;
    }

    List<AddressProvider> getProviders() {
        if (m_providers == null) {
            Map<String, AddressProvider> beanMap = m_beanFactory.getBeansOfType(
                    AddressProvider.class, false, true);
            m_providers = new ArrayList<AddressProvider>(beanMap.values());
        }
        return m_providers;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
