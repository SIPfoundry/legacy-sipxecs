/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import java.io.OutputStream;

import org.apache.commons.io.IOUtils;

public class OutputStreamProfileLocation implements ProfileLocation {

    private OutputStream m_stream;

    public OutputStreamProfileLocation(OutputStream stream) {
        m_stream = stream;
    }

    public OutputStream getOutput(String profileName) {
        return m_stream;
    }

    public void removeProfile(String profileName) {
        // do nothing
    }

    public void closeOutput(OutputStream stream) {
        IOUtils.closeQuietly(stream);
    }
}
