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
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

public class PagingConfiguration implements ConfigProvider {
    private PagingContext m_pagingContext;
    private String m_audioDirectory;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(PagingContext.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        PagingSettings settings = m_pagingContext.getSettings();
        Setting pagingSettings = settings.getSettings().getSetting("page-config");
        String domainName = manager.getDomainManager().getDomainName();
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(PagingContext.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxpage.cfdat", enabled, "sipxpage");
            if (!enabled) {
                continue;
            }

            String log4jFileName = "log4j-page.properties.part";
            SettingUtil.writeLog4jSetting(pagingSettings, dir, log4jFileName);

            FileWriter writer = new FileWriter(new File(dir, "sipxpage.properties.part"));
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
        config.write("sip.address", location.getAddress());
        config.write("rtp.port", settings.getRtpPort());
        config.write("sip.tlsPort", settings.getSipTlsPort());
        config.write("sip.udpPort", settings.getSipUdpPort());
        config.write("sip.tcpPort", settings.getSipTcpPort());
        config.write("sip.trace", settings.getSipTraceLevel());
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

    public String getAudioDirectory() {
        return m_audioDirectory;
    }
}
