/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;

public class ModulesConfiguration extends FreeswitchConfigFile {

    @Override
    protected String getFileName() {
        return "freeswitch/modules.conf.xml";
    }

    @Override
    protected void setupContext(VelocityContext context, Location location, FreeswitchSettings settings) {
        context.put("codecG729", settings.isCodecG729Installed());
    }
}
