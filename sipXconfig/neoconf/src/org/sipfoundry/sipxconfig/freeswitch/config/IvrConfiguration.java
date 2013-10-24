/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.springframework.beans.factory.annotation.Required;

public class IvrConfiguration extends AbstractFreeswitchConfiguration {
    private String m_mediaServer;

    @Override
    protected String getTemplate() {
        return "freeswitch/ivr.conf.xml.vm";
    }

    @Override
    protected String getFileName() {
        return "autoload_configs/ivr.conf.xml";
    }

    @Override
    public void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException {
        write(writer);
    }

    void write(Writer writer) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("mediaserver", m_mediaServer);
        write(writer, context);
    }

    @Required
    public void setMediaServer(String mediaServer) {
        m_mediaServer = mediaServer;
    }
}
