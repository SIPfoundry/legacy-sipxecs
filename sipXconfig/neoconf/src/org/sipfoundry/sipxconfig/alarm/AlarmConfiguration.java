/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.springframework.beans.factory.annotation.Required;

public class AlarmConfiguration implements ConfigProvider {
    private AlarmServerManager m_alarmServerManager;
    private VelocityEngine m_velocityEngine;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(Alarms.FEATURE)) {
            if (manager.getFeatureManager().isFeatureEnabled(Alarms.FEATURE)) {
                List<Alarm> alarms = m_alarmServerManager.getAlarmTypes();
                Location[] locations = manager.getLocationManager().getLocations();
                for (Location location : locations) {
                    File file = new File(manager.getLocationDataDirectory(location), "alarms.xml.cfdat");
                    FileWriter wtr = new FileWriter(file);
                    write(wtr, alarms);
                    IOUtils.closeQuietly(wtr);
                }
            }
        }
    }

    @Required
    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    void write(Writer wtr, List<Alarm> alarms) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("alarms", alarms);
        try {
            m_velocityEngine.mergeTemplate("alarms/alarms.vm", context, wtr);
        } catch (Exception e) {
            throw new IOException(e);
        }
        wtr.flush();
    }
}
