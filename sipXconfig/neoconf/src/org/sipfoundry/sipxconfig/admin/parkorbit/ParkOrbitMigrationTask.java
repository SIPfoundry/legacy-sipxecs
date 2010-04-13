/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.parkorbit;

import java.io.File;
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Orbits;
import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;

public class ParkOrbitMigrationTask extends InitTaskListener {

    private static final Log LOG = LogFactory.getLog(ParkOrbitMigrationTask.class);
    private Orbits m_orbitsGenerator;
    private MusicOnHoldManager m_musicOnHoldManager;

    @Override
    public void onInitTask(String task) {
        LOG.info("Starting park server migration");
        File parkServerDirectory = new File(m_orbitsGenerator.getAudioDirectory());
        File systemMohDirectory = new File(m_musicOnHoldManager.getAudioDirectoryPath());
        try {
            if (parkServerDirectory.exists()) {
                LOG.info("Copying all files from " + m_orbitsGenerator.getAudioDirectory() + " to "
                        + systemMohDirectory.getAbsolutePath());
                FileUtils.copyDirectory(parkServerDirectory, systemMohDirectory);
            }
        } catch (IOException e) {
            LOG.error("Unable to copy files/folder; " + e);
        }
    }

    public Orbits getOrbitsGenerator() {
        return m_orbitsGenerator;
    }

    public void setOrbitsGenerator(Orbits orbitsGenerator) {
        m_orbitsGenerator = orbitsGenerator;
    }

    public MusicOnHoldManager getMusicOnHoldManager() {
        return m_musicOnHoldManager;
    }

    public void setMusicOnHoldManager(MusicOnHoldManager musicOnHoldManager) {
        m_musicOnHoldManager = musicOnHoldManager;
    }

}
