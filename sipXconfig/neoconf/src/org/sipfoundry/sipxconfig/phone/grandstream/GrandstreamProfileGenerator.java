/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.IOException;
import java.io.OutputStream;

import org.sipfoundry.sipxconfig.device.AbstractProfileGenerator;
import org.sipfoundry.sipxconfig.device.ProfileContext;

public class GrandstreamProfileGenerator extends AbstractProfileGenerator {

    protected void generateProfile(ProfileContext context, OutputStream out) throws IOException {
        GrandstreamProfileContext gpc = (GrandstreamProfileContext) context;
        GrandstreamProfileWriter writer = gpc.getWriter();
        writer.write(out);
    }
}
