/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

/**
 * 27117, 27118...
 *
 * stunnel
 * 27117 --> s2:27217
 * 27118 --> s2:27218
 */
public class MongoClientConfig extends TemplateConfigurationFile {

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext c = super.setupContext(location);
        // TODO: multi servers
        c.put("servers", "localhost:27107");
        c.put("replset", "sipxecs");
        return c;
    }
}
