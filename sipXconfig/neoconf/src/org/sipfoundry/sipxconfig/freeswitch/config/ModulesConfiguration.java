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

public class ModulesConfiguration extends AbstractFreeswitchConfiguration {

    @Override
    protected String getFileName() {
        return "autoload_configs/modules.conf.xml";
    }

    @Override
    protected String getTemplate() {
        return "freeswitch/modules.conf.xml.vm";
    }

    @Override
    public void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException {
        write(writer, settings.isCodecG729Installed());
    }

    public void write(Writer writer, boolean g729) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("codecG729", g729);
        write(writer, context);
    }
}
