/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.parkorbit;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

public class ParkOrbitConfiguration implements ConfigProvider, DaoEventListener {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/orbits-00-00";
    private String m_audioDirectory;
    private ParkOrbitContext m_parkOrbitContext;
    private ConfigManager m_configManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(ParkOrbitContext.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        ParkSettings settings = m_parkOrbitContext.getSettings();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(ParkOrbitContext.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxpark.cfdat", enabled, "sipxpark");
            if (!enabled) {
                continue;
            }
            Writer xml = new FileWriter(new File(dir, "orbits.xml"));
            try {
                XmlFile config = new XmlFile(xml);
                config.write(getDocument());
                IOUtils.closeQuietly(xml);
            } finally {
                IOUtils.closeQuietly(xml);
            }

            Writer config = new FileWriter(new File(dir, "sipxpark-config.part"));
            try {
                write(config, location, settings);
            } finally {
                IOUtils.closeQuietly(config);
            }
        }
    }

    void write(Writer writer, Location location, ParkSettings settings) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(writer);
        config.writeSettings(settings.getSettings().getSetting("park-config"));
    }

    Document getDocument() {
        Document document = XmlFile.FACTORY.createDocument();
        Element orbits = document.addElement("orbits", NAMESPACE);
        File dir = new File(m_audioDirectory);
        // add music-on-hold
        Element musicOnHold = orbits.addElement("music-on-hold");
        addBackgroundAudio(musicOnHold, dir, m_parkOrbitContext.getDefaultMusicOnHold());
        // add other orbits
        for (ParkOrbit parkOrbit : m_parkOrbitContext.getParkOrbits()) {
            // ignore disabled orbits
            if (!parkOrbit.isEnabled()) {
                continue;
            }
            Element orbit = orbits.addElement("orbit");
            orbit.addElement("name").setText(parkOrbit.getName());
            orbit.addElement("extension").setText(parkOrbit.getExtension());
            addBackgroundAudio(orbit, dir, parkOrbit.getMusic());
            if (parkOrbit.isParkTimeoutEnabled()) {
                String timeout = Integer.toString(parkOrbit.getParkTimeout());
                orbit.addElement("time-out").setText(timeout);
            }
            if (!parkOrbit.isMultipleCalls()) {
                orbit.addElement("capacity").setText("1");
            }
            if (parkOrbit.isTransferAllowed()) {
                String key = parkOrbit.getTransferKey();
                orbit.addElement("transfer-key").setText(key);
            }
            String description = parkOrbit.getDescription();
            if (StringUtils.isNotBlank(description)) {
                orbit.addElement("description").setText(description);
            }
        }
        return document;
    }

    private void addBackgroundAudio(Element parent, File dir, String music) {
        try {
            File audioFile = new File(dir, music);
            URI uri = new URI("file", StringUtils.EMPTY, audioFile.getAbsolutePath(), null);
            parent.addElement("background-audio").setText(uri.toString());
        } catch (URISyntaxException e) {
            throw new RuntimeException(e);
        }
    }

    public String getAudioDirectory() {
        return m_audioDirectory;
    }

    @Required
    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    @Required
    public void setParkOrbitContext(ParkOrbitContext parkOrbitContext) {
        m_parkOrbitContext = parkOrbitContext;
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof Group) {
            onChange((Group) entity);
        }
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof Group) {
            onChange((Group) entity);
        }
    }

    public void onChange(Group group) {
        if (ParkOrbitContext.PARK_ORBIT_GROUP_ID.equals(group.getResource()) && !group.isNew()) {
            m_configManager.configureEverywhere(ParkOrbitContext.FEATURE);
        }
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }
}
