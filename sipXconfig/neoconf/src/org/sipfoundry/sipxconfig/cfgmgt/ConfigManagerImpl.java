/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;


import java.io.File;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.springframework.beans.factory.annotation.Required;

public class ConfigManagerImpl implements AddressProvider, ConfigManager {

    private File m_cfDataDir;
    private DomainManager m_domainManager;
    private FeatureManager m_featureManager;
    private AddressManager m_addressManager;
    private LocationsManager m_locationManager;

    // thread-safe under theory there could be a lot of thread posting changes
    // not because it was proven to have threading issues as a unsynchronized collection
    private Set<Feature> m_affectedFeatures = Collections.synchronizedSet(new HashSet<Feature>());
    private boolean m_allFeaturesAffected;

    @Override
    public void replicationRequired(Feature feature) {
        // (re)start timer
        m_affectedFeatures.add(feature);
    }

    @Override
    public void replicationRequired(Feature ... features) {
        // (re)start timer
        m_affectedFeatures.addAll(Arrays.asList(features));
    }

    @Override
    public void allFeaturesAffected() {
        m_allFeaturesAffected = true;
    }

    public String getCfDataDir() {
        return m_cfDataDir.getAbsolutePath();
    }

    public void setCfDataDir(String cfDataDir) {
        m_cfDataDir = new File(cfDataDir);
    }

    @Override
    public File getLocationDataDirectory(Location location) {
        return new File(m_cfDataDir, String.valueOf(location.getId()));
    }

    @Override
    public ConfigStatus getStatus(Location location, String key) {
        return ConfigStatus.OK;
    }

    @Override
    public void restartAllJavaProcesses() {
        // TODO
    }

    @Override
    public void restartService(Location location, String service) {
        // TODO
    }

    @Override
    public DomainManager getDomainManager() {
        return m_domainManager;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Override
    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Override
    public AddressManager getAddressManager() {
        return m_addressManager;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Override
    public LocationsManager getLocationManager() {
        return m_locationManager;
    }

    public void setLocationManager(LocationsManager locationManager) {
        m_locationManager = locationManager;
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return Collections.singleton(SUPERVISOR_ADDRESS);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Object requester) {
        if (type.equals(SUPERVISOR_ADDRESS)) {
            // this will eventually phase out in favor of sipxsupervisor-lite
            return Collections.singleton(new Address(null, 8092));
        }
        return null;
    }
}
