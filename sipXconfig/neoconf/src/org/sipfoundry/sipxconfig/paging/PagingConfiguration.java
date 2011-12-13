/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.paging;
import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;

public class PagingConfiguration implements ConfigProvider {
    private PagingContext m_pagingContext;
    private String m_audioDirectory;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(PagingContext.FEATURE)) {
            return;
        }
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(PagingContext.FEATURE);
        PagingSettings settings = m_pagingContext.getSettings();
        String domainName = manager.getDomainManager().getDomainName();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            FileWriter writer = new FileWriter(new File(dir, "sipxpage.properties"));
            KeyValueConfiguration config = new KeyValueConfiguration(writer);
            config.write(settings.getSettings().getSetting("page-config"));
            List<PagingGroup> groups = m_pagingContext.getPagingGroups();
            for (int i = 0; i < groups.size(); i++) {
                PagingGroup g = groups.get(i);
                if (g.isEnabled()) {
                    config.write(i + ".user", g.getPageGroupNumber());
                    config.write(i + ".description", g.getPageGroupNumber());
                    config.write(i + ".urls", g.formatUserList(domainName));
                    String beep = format("file://%s/%s", m_audioDirectory, g.getSound());
                    config.write(i + ".beep", beep);
                    long millis = ((long) g.getTimeout()) * 1000;
                    config.write(i + ".timeout", millis);
                }
            }
        }
    }

    public void setPagingContext(PagingContext pagingContext) {
        m_pagingContext = pagingContext;
    }

    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }
}
