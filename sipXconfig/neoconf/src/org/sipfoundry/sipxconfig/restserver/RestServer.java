/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.restserver;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class RestServer implements FeatureProvider, AddressProvider, ConfigProvider {
    public static final LocationFeature FEATURE = new LocationFeature("resetServer");
    public static final AddressType HTTPS_API = new AddressType("resetServerApi");
    public static final AddressType EXTERNAL_API = new AddressType("resetServerExternalApi");
    public static final AddressType SIP_TCP = new AddressType("resetServerSip");
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(HTTPS_API, EXTERNAL_API, SIP_TCP);
    private BeanWithSettingsDao<RestServerSettings> m_settingsDao;
    private VelocityEngine m_velocityEngine;

    public RestServerSettings getSettings() {
        return m_settingsDao.findOne();
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
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return ADDRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Object requester) {
        List<Address> addresses = null;
        if (ADDRESSES.contains(type)) {
            RestServerSettings settings = getSettings();
            Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
            addresses = new ArrayList<Address>(locations.size());
            for (Location location : locations) {
                Address address = new Address();
                address.setAddress(location.getAddress());
                if (type.equals(HTTPS_API)) {
                    address.setPort(settings.getHttpsPort());
                    address.setFormat("https://%s:%d");
                } else if (type.equals(EXTERNAL_API)) {
                    address.setPort(settings.getExternalPort());
                } else if (type.equals(SIP_TCP)) {
                    address.setPort(settings.getSipPort());
                }
                addresses.add(address);
            }
        }

        return addresses;
    }

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!manager.getFeatureManager().isFeatureEnabled(FEATURE)) {
            return;
        }

        RestServerSettings settings = getSettings();
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer wtr = new FileWriter(new File(dir, "sipxrest-config.xml"));
            try {
                VelocityContext context = new VelocityContext();
                context.put("settings", settings.getSettings().getSetting("rest-config"));
                context.put("location", location);
                context.put("domainName", manager.getDomainManager().getDomainName());
                try {
                    m_velocityEngine.mergeTemplate("sipxrest/sipxrest-config.vm", context, wtr);
                } catch (Exception e) {
                    throw new IOException(e);
                }
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    public void setSettingsDao(BeanWithSettingsDao<RestServerSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
}
