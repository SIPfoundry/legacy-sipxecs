/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.LoggerKeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.springframework.beans.factory.annotation.Required;

public class HzConfig implements ConfigProvider {
    private static final Log LOG = LogFactory.getLog(HzConfig.class);

    private VelocityEngine m_velocityEngine;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(AdminContext.FEATURE)) {
            return;
        }

        VelocityContext context = new VelocityContext();
        AddressManager addressManager = manager.getAddressManager();

        List<Address> addresses = addressManager.getAddresses(Ivr.REST_API);
        addresses.add(addressManager.getSingleAddress(AdminContext.HTTP_ADDRESS));

        //ensure unique address
        context.put("addresses", new HashSet<Address>(addresses));

        Set<Location> locations = request.locations(manager);
        FeatureManager featureManager = manager.getFeatureManager();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean ivrEnabled = featureManager.isFeatureEnabled(Ivr.FEATURE, location);
            if (location.isPrimary() || ivrEnabled) {
                LOG.debug("Replicate hz configuration to: " + location.getAddress());
                File f = new File(dir, "hz-config.xml");
                Writer wtr = new FileWriter(f);
                try {
                    m_velocityEngine.mergeTemplate("hz-config.vm", context, wtr);
                } finally {
                    IOUtils.closeQuietly(wtr);
                }
            }
        }
    }

    void write(Writer wtr, List<String> hzAddresses) throws IOException {
        LoggerKeyValueConfiguration config = LoggerKeyValueConfiguration.equalsSeparated(wtr);
        config.write("hz.addresses", hzAddresses);
    }

    @Required
    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
}
