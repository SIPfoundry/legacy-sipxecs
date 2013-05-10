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

import static org.apache.commons.lang.StringUtils.join;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;

public class OpenAcdConfiguration implements ConfigProvider, FeatureListener, BeanFactoryAware {
    private static final String COMMA = ",";
    private static final String NEW_LINE = "\n";
    private static final String CLOSE = "]}";
    private static final String CLOSE_COMMA = "]},";
    private OpenAcdContext m_openAcdContext;
    private OpenAcdReplicationProvider m_openAcdReplicationProvider;
    private SipxReplicationContext m_sipxReplicationContext;
    private ListableBeanFactory m_beanFactory;

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

            if (enabled) {
                Writer wtr = new FileWriter(new File(dir, "sys.config.part"));
                Map<String, OpenAcdConfigurationPlugin> plugins = m_beanFactory
                        .getBeansOfType(OpenAcdConfigurationPlugin.class);
                List<String> pluginDefinitions = new ArrayList<String>();
                List<String> pluginConfigurations = new ArrayList<String>();
                for (OpenAcdConfigurationPlugin bean : plugins.values()) {
                    pluginDefinitions.add(bean.getOpenAcdPluginName());
                    pluginConfigurations.add(bean.getOpenAcdPluginConfiguration());
                }
                try {
                    writeConfigFile(wtr, pluginDefinitions, pluginConfigurations);
                } finally {
                    IOUtils.closeQuietly(wtr);
                }
            }
        }
    }

    void writeConfigFile(Writer wtr, List<String> pluginDefinitions, List<String> pluginConfigurations)
        throws IOException {
        wtr.write("[{openacd, [");
        wtr.write(NEW_LINE);
        wtr.write("  {rsakey, \"$(sipx.OPENACD_KEYDIR)/openacd.key\"}");
        wtr.write(NEW_LINE);
        wtr.write("    %% Add plugins here");
        wtr.write(NEW_LINE);
        if (pluginDefinitions.isEmpty()) {
            wtr.write("    , {plugins, [oacd_freeswitch, oacd_web, oacd_dialplan, oacd_spx]}");
        } else {
            wtr.write("    , {plugins, [oacd_freeswitch, oacd_web, oacd_dialplan, oacd_spx, "
                    + join(pluginDefinitions.toArray(), COMMA) + CLOSE);
        }
        wtr.write(NEW_LINE);
        wtr.write("    , {agent_auth_storage, spx_agent_auth}");
        wtr.write(NEW_LINE);
        wtr.write("    , {call_queue_config_storage, spx_call_queue_config}");
        wtr.write(NEW_LINE);
        wtr.write(CLOSE_COMMA);
        wtr.write(NEW_LINE);
        wtr.write("%% Application-specific configuration here");
        wtr.write(NEW_LINE);
        wtr.write("{oacd_freeswitch, [");
        wtr.write(NEW_LINE);
        wtr.write("    {freeswitch_node, 'freeswitch@127.0.0.1'},");
        wtr.write(NEW_LINE);
        wtr.write("    {cpx_managed, true}");
        wtr.write(NEW_LINE);
        wtr.write(CLOSE_COMMA);
        wtr.write(NEW_LINE);
        for (String configuration : pluginConfigurations) {
            wtr.write(configuration);
            wtr.write(COMMA);
            wtr.write(NEW_LINE);
        }
        wtr.write("{lager, [");
        wtr.write(NEW_LINE);
        wtr.write("    {handlers, [");
        wtr.write(NEW_LINE);
        wtr.write("        {lager_console_backend, [info, true]},");
        wtr.write(NEW_LINE);
        wtr.write("        {lager_file_backend, [");
        wtr.write(NEW_LINE);
        wtr.write("            {\"$(sipx.OPENACD_LOGDIR)/error.log\", error, 10485760, \"$D0\", 5},");
        wtr.write(NEW_LINE);
        wtr.write("            {\"$(sipx.OPENACD_LOGDIR)/console.log\", $(sipx.OPENACD_LOG_LEVEL), "
                + "10485760, \"$D0\", 5}");
        wtr.write(NEW_LINE);
        wtr.write("        ]}");
        wtr.write(NEW_LINE);
        wtr.write("    ]}");
        wtr.write(NEW_LINE);
        wtr.write(CLOSE);
        wtr.write(NEW_LINE);
        wtr.write("].");
    }

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

    @Required
    public void setOpenAcdReplicationProvider(OpenAcdReplicationProvider openAcdReplicationProvider) {
        m_openAcdReplicationProvider = openAcdReplicationProvider;
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        if (request.getAllNewlyEnabledFeatures().contains(OpenAcdContext.FEATURE)) {
            for (Replicable openAcdObject : m_openAcdReplicationProvider.getReplicables()) {
                m_sipxReplicationContext.generate(openAcdObject);
            }
        }
    }
}
