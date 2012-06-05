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
package org.sipfoundry.sipxconfig.logwatcher;


import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;

public class LogWatcherConfig implements ConfigProvider {
    private LogWatcher m_logWatcher;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(LogWatcher.FEATURE)) {
            return;
        }

        File gdir = manager.getGlobalDataDirectory();
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(LogWatcher.FEATURE);
        LogWatcherSettings settings = m_logWatcher.getSettings();
        Writer w = new FileWriter(new File(gdir, "sipxlogwatcher.cfdat"));
        try {
            CfengineModuleConfiguration cfg = new CfengineModuleConfiguration(w);
            cfg.writeClass(LogWatcher.FEATURE.getId(), enabled);
            cfg.writeSettings("logwatcher_", settings.getSettings().getSetting("config"));
        } finally {
            IOUtils.closeQuietly(w);
        }
    }

    public void setLogWatcher(LogWatcher logWatcher) {
        m_logWatcher = logWatcher;
    }
}
