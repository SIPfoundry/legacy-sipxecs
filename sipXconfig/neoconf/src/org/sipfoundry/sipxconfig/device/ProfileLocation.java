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
 * Generic location for profiles.
 */
public interface ProfileLocation {
    /**
     * Called to obtain the output stream that might be used to write profile content.
     *
     * @param profileName identifies profile to be propagated
     * @return output stream, write to it and call closeOutput when done
     */
    OutputStream getOutput(String profileName);

    /**
     * Called after the entire content has been written to the stream.
     *
     * Good place to close resources, copy files etc.
     *
     * @param stream stream previously retrieved using getOutput
     */
    void closeOutput(OutputStream stream);

    void removeProfile(String profileName);
}
