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


public class LogfileConfiguration extends AbstractFreeswitchConfiguration {

    @Override
    protected String getTemplate() {
        return "freeswitch/logfile.conf.xml.vm";
    }

    @Override
    public void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("settings", settings.getSettings().getSetting("freeswitch-config"));
        write(writer, context);
    }

    @Override
    protected String getFileName() {
        return "autoload_configs/logfile.conf.xml";
    }
}
