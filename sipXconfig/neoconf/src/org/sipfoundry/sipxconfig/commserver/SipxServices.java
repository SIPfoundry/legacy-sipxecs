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
package org.sipfoundry.sipxconfig.commserver;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.annotation.Required;

public class SipxServices implements SetupListener, FeatureListener, ConfigProvider {
    private static final Log LOG = LogFactory.getLog(SipxServices.class);
    private ConfigManager m_configManager;
    private SnmpManager m_snmpManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies()) {
            return;
        }

        writerServicesList();
    }

    @Override
    public boolean setup(SetupManager manager) {
        String id = "sipxecs-services";
        if (!manager.isTrue(id)) {
            writerServicesList();
            manager.setTrue(id);
        }
        return true;
    }

    void writerServicesList() {
        Location[] all = m_configManager.getLocationManager().getLocations();
        for (Location location : all) {
            FileWriter services = null;
            File dir = m_configManager.getLocationDataDirectory(location);
            try {
                services = new FileWriter(new File(dir, "sipxecs-services.ini"));
                for (ProcessDefinition process : m_snmpManager.getProcessDefinitions(location)) {
                    if (!process.isHideFromGlobalServiceScript()) {
                        String processName = process.getProcessMask();
                        services.write(processName);
                        services.write("\n");
                    }
                }
            } catch (IOException e) {
                LOG.error("Cannot create/write sipxecs-services.ini file: " + e.getMessage());
            } finally {
                IOUtils.closeQuietly(services);
            }
        }
    }

    @Required
    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Required
    public void setSnmpManager(SnmpManager snmpManager) {
        m_snmpManager = snmpManager;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        writerServicesList();
    }
}
