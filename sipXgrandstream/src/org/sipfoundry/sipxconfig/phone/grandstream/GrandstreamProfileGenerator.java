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

import java.io.OutputStream;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;

public class GrandstreamProfileGenerator extends VelocityProfileGenerator {

    protected void generateProfile(ProfileContext context, OutputStream out) {
        if (context instanceof GrandstreamProfileContext) {
            GrandstreamProfileContext gpc = (GrandstreamProfileContext) context;
            GrandstreamProfileWriter writer = gpc.getWriter();
            writer.write(out);
        } else if (context instanceof GrandstreamPhonebook) {
            super.generateProfile(context, out);
        }
    }

}
