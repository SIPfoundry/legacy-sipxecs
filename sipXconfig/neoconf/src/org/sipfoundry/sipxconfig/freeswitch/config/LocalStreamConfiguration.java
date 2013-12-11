/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import java.io.IOException;
import java.io.Writer;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.springframework.beans.factory.annotation.Required;

public class LocalStreamConfiguration extends AbstractFreeswitchConfiguration {
    private MusicOnHoldManager m_musicOnHoldManager;
    private String m_docDir;

    @Required
    public void setMusicOnHoldManager(MusicOnHoldManager musicOnHoldManager) {
        m_musicOnHoldManager = musicOnHoldManager;
    }

    public void setDocDir(String docDir) {
        m_docDir = docDir;
    }

    @Override
    public void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException {
        String audioDir = null;
        if (!m_musicOnHoldManager.isAudioDirectoryEmpty()) {
            audioDir =  m_musicOnHoldManager.getAudioDirectoryPath();
        }
        write(writer, audioDir, m_musicOnHoldManager.isMohHD());
    }

    void write(Writer writer, String audioDir, boolean hd) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("moh", m_musicOnHoldManager);
        context.put("hd", hd);
        context.put("audioDir", audioDir);
        context.put("docDir", m_docDir);
        write(writer, context);
    }

    @Override
    protected String getTemplate() {
        return "freeswitch/local_stream.conf.xml.vm";
    }

    @Override
    protected String getFileName() {
        return "autoload_configs/local_stream.conf.xml";
    }
}
