/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Map;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class FreeswitchConfigurationProvider implements ConfigProvider, BeanFactoryAware {
    private FreeswitchFeature m_freeswitch;
    private ListableBeanFactory m_beanFactory;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FreeswitchFeature.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(FreeswitchFeature.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxfreeswitch.cfdat", enabled, "sipxfreeswitch");
            if (!enabled) {
                continue;
            }
            FreeswitchSettings settings = m_freeswitch.getSettings(location);
            Map<String, AbstractFreeswitchConfiguration> configs = m_beanFactory
                    .getBeansOfType(AbstractFreeswitchConfiguration.class);
            for (AbstractFreeswitchConfiguration config : configs.values()) {
                File f = new File(dir, config.getFileName());
                f.getParentFile().mkdirs();
                FileWriter writer = new FileWriter(f);
                config.write(writer, location, settings);
                IOUtils.closeQuietly(writer);
            }
        }
    }

    public void setFreeswitch(FreeswitchFeature freeswitch) {
        m_freeswitch = freeswitch;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
}
