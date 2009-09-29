/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public interface ProfileFilter {
    void copy(InputStream in, OutputStream out) throws IOException;
}
