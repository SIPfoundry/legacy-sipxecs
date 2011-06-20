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

import org.sipfoundry.sipxconfig.device.ProfileContext;

public class GrandstreamProfileContext extends ProfileContext {
    private GrandstreamProfileWriter m_writer;

    public GrandstreamProfileContext(GrandstreamPhone device, boolean isTextFormatEnabled) {
        super(device, null);
        if (isTextFormatEnabled) {
            m_writer = new GrandstreamProfileWriter(device);
        } else {
            m_writer = new GrandstreamBinaryProfileWriter(device);
        }
    }

    public GrandstreamProfileWriter getWriter() {
        return m_writer;
    }
}
