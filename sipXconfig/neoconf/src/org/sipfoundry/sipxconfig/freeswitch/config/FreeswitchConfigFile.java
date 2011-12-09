/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import java.io.IOException;
import java.io.Writer;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;

public abstract class FreeswitchConfigFile {
    private VelocityEngine m_velocityEngine;
    private FreeswitchFeature m_freeswitchFeature;

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException {
        VelocityContext context = new VelocityContext();
        setupContext(context, location, settings);
        try {
            m_velocityEngine.mergeTemplate(getTemplate(), context, writer);
        } catch (Exception e) {
            throw new IOException(e);
        }
    }

    protected String getTemplate() {
        return getFileName() + ".vm";
    }

    protected abstract String getFileName();

    protected abstract void setupContext(VelocityContext context, Location location,
            FreeswitchSettings settings);

    public FreeswitchFeature getFreeswitchFeature() {
        return m_freeswitchFeature;
    }

    public void setFreeswitchFeature(FreeswitchFeature freeswitchFeature) {
        m_freeswitchFeature = freeswitchFeature;
    }

}
