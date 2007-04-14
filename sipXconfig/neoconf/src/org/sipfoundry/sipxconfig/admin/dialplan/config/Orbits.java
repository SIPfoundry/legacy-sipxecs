/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.File;
import java.io.IOException;
import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.parkorbit.BackgroundMusic;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit;

public class Orbits extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/orbits-00-00";
    private static final String FILENAME = "orbits.xml";

    private String m_configDirectory;

    private String m_audioDirectory;

    private Document m_document;

    public Document getDocument() {
        return m_document;
    }

    public void generate(BackgroundMusic defaultMusic, Collection<ParkOrbit> parkOrbits) {
        m_document = FACTORY.createDocument();
        Element orbits = m_document.addElement("orbits", NAMESPACE);
        File dir = new File(m_audioDirectory);
        // add music-on-hold
        Element musicOnHold = orbits.addElement("music-on-hold");
        addBackgroundAudio(musicOnHold, dir, defaultMusic);
        // add other orbits
        for (ParkOrbit parkOrbit : parkOrbits) {
            // ignore disabled orbits
            if (!parkOrbit.isEnabled()) {
                continue;
            }
            Element orbit = orbits.addElement("orbit");
            orbit.addElement("name").setText(parkOrbit.getName());
            orbit.addElement("extension").setText(parkOrbit.getExtension());
            addBackgroundAudio(orbit, dir, parkOrbit);
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
    }

    private void addBackgroundAudio(Element parent, File dir, BackgroundMusic music) {
        File audioFile = new File(dir, music.getMusic());
        parent.addElement("background-audio").setText("file://" + audioFile.getAbsolutePath());
    }

    /**
     * Writes to file in a specified directory
     * 
     * @throws IOException
     */
    public void writeToFile() throws IOException {
        File parent = new File(m_configDirectory);
        writeToFile(parent, FILENAME);
    }

    public String getAudioDirectory() {
        return m_audioDirectory;
    }

    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public ConfigFileType getType() {
        return ConfigFileType.ORBITS;
    }
}
