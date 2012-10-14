/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver;

import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.springframework.context.ApplicationEvent;

public interface SipxReplicationContext {
    public static final String IMDB_REGENERATION = "IMDB regeneration";
    public static final String MONGO_LOCATION_REGISTRATION = "Location registration in IMDB";
    void generateAll();
    void generateAll(DataSet ds);

    /**
     * This function will publish application event - in case the application is done lazily it
     * will publish the event only after everything has been replicated
     *
     * @param event event to be published
     */
    void publishEvent(ApplicationEvent event);

    void generate(Replicable entity);
    void remove(Replicable entity);
    void replicateWork(Replicable entity);
}
