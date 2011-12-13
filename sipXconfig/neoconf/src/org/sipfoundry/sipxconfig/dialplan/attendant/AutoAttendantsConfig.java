/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.dialplan.attendant;

import static org.sipfoundry.sipxconfig.dialplan.AutoAttendantManager.ATTENDANT_GROUP_ID;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.common.event.EntitySaveListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.setting.Group;

public class AutoAttendantsConfig extends EntitySaveListener<Group> implements ConfigProvider {
    private ConfigManager m_configManager;
    private AutoAttendantXmlConfig m_xml;

    public AutoAttendantsConfig() {
        super(Group.class);
    }

    @Override
    protected void onEntitySave(Group group) {
        if (ATTENDANT_GROUP_ID.equals(group.getResource()) && !group.isNew()) {
            m_configManager.replicationRequired(AutoAttendants.FEATURE, Ivr.FEATURE);
        }
    }

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(DialPlanContext.FEATURE, AutoAttendants.FEATURE, LocalizationContext.FEATURE)
                || !request.isFirstTime(this)) {
            return;
        }

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(AutoAttendants.FEATURE);
        Document doc = m_xml.getDocument();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            FileWriter wtr = new FileWriter(new File(dir, "autoattendants.xml"));
            XmlFile config = new XmlFile(wtr);
            config.write(doc);
            IOUtils.closeQuietly(wtr);
        }
        request.firstTimeOver(this);
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setXml(AutoAttendantXmlConfig xml) {
        m_xml = xml;
    }
}

