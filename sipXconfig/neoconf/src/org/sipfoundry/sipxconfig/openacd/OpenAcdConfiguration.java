/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.openacd;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public class OpenAcdConfiguration implements ConfigProvider, FeatureListener {
    private VelocityEngine m_velocityEngine;
    private OpenAcdContext m_openAcdContext;
    private OpenAcdProvisioningContext m_provisioningContext;

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
        if (!feature.equals(OpenAcdContext.FEATURE)) {
            return;
        }

        if (feature.equals(OpenAcdContext.FEATURE)) {
            if (event == FeatureEvent.PRE_ENABLE) {
                OpenAcdSettings settings = m_openAcdContext.getSettings();
                m_provisioningContext.configure(settings.getConfigObjects());
            }
        }
    }

    @Override
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
    }

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(OpenAcdContext.FEATURE)) {
            return;
        }

        OpenAcdSettings settings = m_openAcdContext.getSettings();
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(OpenAcdContext.FEATURE);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer vmArgs = new FileWriter(new File(dir, "vm.args"));
            try {
                VelocityContext context = new VelocityContext();
                context.put("fqdn", location.getFqdn());
                try {
                    m_velocityEngine.mergeTemplate("sipxopenacd/vm.args.vm", context, vmArgs);
                } catch (Exception e) {
                    throw new IOException(e);
                }
            } finally {
                IOUtils.closeQuietly(vmArgs);
            }

            Writer app = new FileWriter(new File(dir, "app.config"));
            try {
                VelocityContext context = new VelocityContext();
                context.put("log_level", settings.getLogLevel());
                try {
                    m_velocityEngine.mergeTemplate("sipxopenacd/app.config.vm", context, app);
                } catch (Exception e) {
                    throw new IOException(e);
                }
            } finally {
                IOUtils.closeQuietly(app);
            }
        }
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

    public void setProvisioningContext(OpenAcdProvisioningContext provisioningContext) {
        m_provisioningContext = provisioningContext;
    }
}
