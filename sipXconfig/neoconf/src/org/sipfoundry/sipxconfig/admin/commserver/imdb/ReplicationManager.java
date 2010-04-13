/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

/**
 * Interface to replication.cgi
 */
public interface ReplicationManager {
    /**
     * Replicates IMDB data sets to remore locations
     *
     * @param locations list of locations that will receive replicated data
     * @param generator data set to be replicated
     * @return true if the replication has been successful, false otherwise
     */
    boolean replicateData(Location[] locations, DataSetGenerator generator);

    /**
     * Replicates file content to remore locations
     *
     * @param locations list of locations that will receive replicated file
     * @param file object representing file content
     *
     * @return true if the replication has been successful, false otherwise
     */
    boolean replicateFile(Location[] locations, ConfigurationFile file);
}
