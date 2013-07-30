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
package org.sipfoundry.sipxconfig.freeswitch.config;

import java.io.IOException;
import java.io.Writer;
import java.util.Collections;
import java.util.List;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;

public abstract class AbstractFreeswitchConfiguration implements FreeswitchProvider {
    private VelocityEngine m_velocityEngine;
    private FreeswitchFeature m_freeswitchFeature;

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public abstract void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException;

    protected void write(Writer writer, VelocityContext context) throws IOException {
        try {
            m_velocityEngine.mergeTemplate(getTemplate(), context, writer);
        } catch (Exception e) {
            throw new IOException(e);
        }
    }

    protected abstract String getTemplate();

    protected abstract String getFileName();

    /**
     * Override to add modules to modules.conf.xml. Note you don't have to have a subclass
     * of this class to add modules. Any bean that implements FreeswitchProvider can
     * also submit modules to be enabled.
     *
     * @return list of module names.
     */
    @Override
    public List<String> getRequiredModules(FreeswitchFeature feature, Location location) {
        return Collections.emptyList();
    }

    public FreeswitchFeature getFreeswitchFeature() {
        return m_freeswitchFeature;
    }

    public void setFreeswitchFeature(FreeswitchFeature freeswitchFeature) {
        m_freeswitchFeature = freeswitchFeature;
    }
}
