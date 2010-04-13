/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.springframework.context.ApplicationEvent;

public interface SipxReplicationContext {
    void generate(DataSet dataSet);

    void generateAll();

    /**
     * Replicates configuration to all locations.
     *
     * @param conf configuration to be replicated
     */
    void replicate(ConfigurationFile conf);

    /**
     * Replicates configuration to a specific location only...
     *
     * @param location server to which location needs to be pushed
     * @param conf configuration to be replicated
     */
    void replicate(Location location, ConfigurationFile conf);

    String getXml(DataSet dataSet);

    /**
     * This function will publish application event - in case the application is done lazily it
     * will publish the even only after everything has been replicated
     *
     * @param event event to be published
     */
    void publishEvent(ApplicationEvent event);
}
