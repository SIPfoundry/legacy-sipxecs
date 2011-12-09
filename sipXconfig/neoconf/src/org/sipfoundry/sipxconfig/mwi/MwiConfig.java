/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mwi;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.springframework.beans.factory.annotation.Required;

public class MwiConfig implements ConfigProvider {
    private Mwi m_mwi;
    private VelocityEngine m_velocityEngine;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Mwi.FEATURE)) {
            return;
        }

        Address ivrApi = manager.getAddressManager().getSingleAddress(Ivr.REST_API);
        Location[] allLocations = manager.getLocationManager().getLocations();
        StringBuilder validIps = new StringBuilder(allLocations[0].getAddress());
        for (int i = 1; i < validIps.length(); i++) {
            validIps.append(",").append(allLocations[i].getAddress());
        }
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(Mwi.FEATURE);
        for (Location location : locations) {
            MwiSettings settings = m_mwi.getSettings();
            File dir = manager.getLocationDataDirectory(location);
            File file = new File(dir, "status-config.cfdat");
            Writer wtr = new FileWriter(file);
            try {
                KeyValueConfiguration config = new KeyValueConfiguration(wtr);
                config.write(settings.getSettings().getSetting("status-config"));
                DomainManager dm = manager.getDomainManager();
                config.write("SIP_STATUS_BIND_IP", location.getAddress());
                config.write("SIP_STATUS_AUTHENTICATE_REALM", dm.getAuthorizationRealm());
                config.write("SIP_STATUS_DOMAIN_NAME", dm.getDomainName());
                config.write("SIP_STATUS_HTTP_VALID_IPS", validIps.toString());
            } finally {
                IOUtils.closeQuietly(wtr);
            }

            Writer plugin = new FileWriter(new File(dir, "status-plugin.xml"));
            try {
                VelocityContext context = new VelocityContext();
                context.put("mwiUrl", ivrApi.toString() + "/mwi");
                try {
                    m_velocityEngine.mergeTemplate("sipxstatus/status-plugin.vm", context, plugin);
                } catch (Exception e) {
                    throw new IOException(e);
                }
            } finally {
                IOUtils.closeQuietly(plugin);
            }
        }
    }

    @Required
    public void setMwi(Mwi mwi) {
        m_mwi = mwi;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
}
