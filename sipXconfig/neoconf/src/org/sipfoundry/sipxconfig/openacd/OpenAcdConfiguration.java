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
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.PostConfigListener;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.SipxReplicationContext;

public class OpenAcdConfiguration implements ConfigProvider, PostConfigListener {
    private OpenAcdContext m_openAcdContext;
    private OpenAcdReplicationProvider m_openAcdReplicationProvider;
    private SipxReplicationContext m_sipxReplicationContext;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(OpenAcdContext.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        if (locations.isEmpty()) {
            return;
        }

        OpenAcdSettings settings = m_openAcdContext.getSettings();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(OpenAcdContext.FEATURE, location);
            Writer w = new FileWriter(new File(dir, "sipxopenacd.cfdat"));
            try {
                CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
                config.writeClass("sipxopenacd", enabled);
                config.write("OPENACD_LOG_LEVEL", settings.getLogLevel());
            } finally {
                IOUtils.closeQuietly(w);
            }
        }
    }

    @Override
    public void postReplicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(OpenAcdContext.FEATURE)) {
            for (Replicable openAcdObject : m_openAcdReplicationProvider.getReplicables()) {
                m_sipxReplicationContext.generate(openAcdObject);
            }
        }
    }

    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

    public void setOpenAcdReplicationProvider(OpenAcdReplicationProvider openAcdReplicationProvider) {
        m_openAcdReplicationProvider = openAcdReplicationProvider;
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

}
