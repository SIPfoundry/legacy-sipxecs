/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.springframework.beans.factory.annotation.Required;

public class LocalStreamConfiguration extends FreeswitchConfigFile {
    private MusicOnHoldManager m_musicOnHoldManager;

    @Required
    public void setMusicOnHoldManager(MusicOnHoldManager musicOnHoldManager) {
        m_musicOnHoldManager = musicOnHoldManager;
    }

    @Override
    protected String getTemplate() {
        return getFileName() + ".vm";
    }

    @Override
    protected void setupContext(VelocityContext context, Location location, FreeswitchSettings settings) {
        context.put("moh", m_musicOnHoldManager);
    }

    @Override
    protected String getFileName() {
        return "freeswitch/local_stream.conf.xml";
    }
}
