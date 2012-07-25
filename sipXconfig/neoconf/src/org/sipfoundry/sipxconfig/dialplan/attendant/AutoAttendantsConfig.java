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
package org.sipfoundry.sipxconfig.dialplan.attendant;

import static org.sipfoundry.sipxconfig.dialplan.AutoAttendantManager.ATTENDANT_GROUP_ID;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.common.event.EntitySaveListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.setting.Group;

public class AutoAttendantsConfig  extends EntitySaveListener<Group> implements ConfigProvider {
    private ConfigManager m_configManager;
    private AutoAttendantXmlConfig m_xml;

    public AutoAttendantsConfig() {
        super(Group.class);
    }

    @Override
    protected void onEntitySave(Group group) {
        if (ATTENDANT_GROUP_ID.equals(group.getResource()) && !group.isNew()) {
            m_configManager.configureEverywhere(AutoAttendantManager.FEATURE, Ivr.FEATURE);
        }
    }

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(DialPlanContext.FEATURE, AutoAttendantManager.FEATURE, Ivr.FEATURE,
                LocalizationContext.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(Ivr.FEATURE, location);
            if (!enabled) {
                continue;
            }

            Document doc = m_xml.getDocument();
            FileWriter wtr = new FileWriter(new File(dir, "autoattendants.xml"));
            XmlFile config = new XmlFile(wtr);
            config.write(doc);
            IOUtils.closeQuietly(wtr);
        }
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setXml(AutoAttendantXmlConfig xml) {
        m_xml = xml;
    }
}
