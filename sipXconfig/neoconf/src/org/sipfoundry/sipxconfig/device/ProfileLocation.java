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

import java.io.OutputStream;

/**
 * Genereric location for profiles. Provides a writer that should be closed once the generation is
 * done.
 */
public interface ProfileLocation {
    OutputStream getOutput(String profileName);

    void removeProfile(String profileName);
}
