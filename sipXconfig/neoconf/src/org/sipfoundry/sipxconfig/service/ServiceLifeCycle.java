/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

/**
 * Services can implement these methods to add custom processing.
 */
public interface ServiceLifeCycle {

    void onInit();

    void onDestroy();

    void onStart();

    void onStop();

    void onRestart();

    /**
     * Called when service settings are saved to DB.
     */
    void onConfigChange();

    /**
     * Called after configuration files replication for a service succeeds
     *
     * @param location if null service configuration was replicated to all locations on which
     *        service is installed
     */
    void afterReplication(Location location);
}
