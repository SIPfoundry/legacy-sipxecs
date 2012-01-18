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
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
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
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            FileWriter writer = new FileWriter(new File(dir, "sipxpage.properties"));
            try {
                write(writer, location, groups, settings, domainName);
            } finally {
                IOUtils.closeQuietly(writer);
            }
        }
    }

    void write(Writer writer, Location location, List<PagingGroup> groups, PagingSettings settings, String domainName)
        throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(writer);
        config.write(settings.getSettings().getSetting("page-config"));
        config.write("sip.address", location.getAddress());
        for (int i = 0; i < groups.size(); i++) {
            PagingGroup g = groups.get(i);
            if (g.isEnabled()) {
                String prefix = "page.group." + (i + 1);
                config.write(prefix + ".user", g.getPageGroupNumber());
                config.write(prefix  + ".description", StringUtils.defaultString(g.getDescription()));
                config.write(prefix + ".urls", g.formatUserList(domainName));
                String beep = format("file://%s/%s", m_audioDirectory, g.getSound());
                config.write(prefix + ".beep", beep);
                long millis = ((long) g.getTimeout()) * 1000;
                config.write(prefix + ".timeout", millis);
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
