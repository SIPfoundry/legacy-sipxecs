/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public interface ConfigurationFile {
    /**
     * Writes document to specified writer
     *
     * @param writer destination for file content.
     * @throws IOException
     */
    void write(Writer writer, Location location) throws IOException;

    /**
     * Returns the name of the file.
     *
     * This is just the last name in the pathname's name sequence.
     */
    String getName();

    /**
     * Return fully qualified name of the configuration file
     *
     * @return full path of the configuration file on the destination system
     */
    String getPath();

    /**
     * Verifies if this configuration file can be replicated on the given location
     *
     * @param location
     * @return true if the file can be replicated - false otherwise
     */
    boolean isReplicable(Location location);

    /**
     * Checks if a service need to be restarted when this configuration file is changed
     *
     * @return true if a service need to be restarted - false otherwise
     */
    boolean isRestartRequired();
}
