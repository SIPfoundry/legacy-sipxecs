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

public class ConfigManager implements AddressProvider {
    public static final AddressType SUPERVISOR_ADDRESS = new AddressType("supervisorXmlRpc");
    public enum ConfigStatus {
        OK, IN_PROGRESS, FAILED
    }

    private File m_cfDataDir;
    private DomainManager m_domainManager;
    private FeatureManager m_featureManager;
    private AddressManager m_addressManager;
    private LocationsManager m_locationManager;

    // thread-safe under theory there could be a lot of thread posting changes
    // not because it was proven to have threading issues as a unsynchronized collection
    private Set<Feature> m_affectedFeatures = Collections.synchronizedSet(new HashSet<Feature>());

    /**
     * Denote a change in objects related to this feature. Normally code would not have to call
     * this explicitly, but objects should simple implement DeployConfigOnEdit and this call would
     * be made for them.
     *
     * @param feature affected area
     */
    public void replicationRequired(Feature feature) {
        // (re)start timer
        m_affectedFeatures.add(feature);
    }

    public void replicationRequired(Feature ... features) {
        // (re)start timer
        m_affectedFeatures.addAll(Arrays.asList(features));
    }

    public String getCfDataDir() {
        return m_cfDataDir.getAbsolutePath();
    }

    public void setCfDataDir(String cfDataDir) {
        m_cfDataDir = new File(cfDataDir);
    }

    public File getLocationDataDirectory(Location location) {
        return new File(m_cfDataDir, String.valueOf(location.getId()));
    }

    /**
     * cfengine promises can emit an unbounded lists of health status identified by a unique key
     * controlled within the promise itself.
     */
    public ConfigStatus getStatus(Location location, String key) {
        return ConfigStatus.OK;
    }

    /**
     * Denote something has changed that would affect all java related services (e.g. certificate
     * change)
     */
    public void restartAllJavaProcesses() {
        // TODO
    }

    /**
     * For whatever reason, you want to explicitly restart a particular service
     */
    public void restartService(Location location, String service) {
        // TODO
    }

    public DomainManager getDomainManager() {
        return m_domainManager;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public AddressManager getAddressManager() {
        return m_addressManager;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

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
