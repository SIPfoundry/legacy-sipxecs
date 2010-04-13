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

import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public interface ServiceConfigurator {

    /**
     * Starts service on a specific location.
     *
     * Before service can be started we need to generate its configuration sets configuration
     * stamp and finally start it.
     *
     * @param location server on which service should be started
     * @param service service to be started
     */
    public void startService(Location location, SipxService service);

    void replicateServiceConfig(SipxService service);

    void replicateServiceConfig(SipxService service, boolean noRestartOnly);

    void replicateServiceConfig(Location location, SipxService sipxService);

    void replicateServiceConfig(Location location, SipxService sipxService, boolean noRestartOnly);

    void replicateServiceConfig(Collection<SipxService> services);

    /**
     * Replicates configuration for all the services on a specific location.
     *
     * @param locationToActivate
     */
    public void replicateLocation(Location location);

    /**
     * Replicates configuration for all the services for all the locations
     */
    void replicateAllServiceConfig();

    /**
     * Modifies list of services running an a specified location to reflect the bundles selection.
     *
     * If the list of processes running on the server correctly reflects the selected bundle none
     * of them will be started (or stopped).
     *
     * @param location server on which to start/stop services
     */
    public void enforceRole(Location location);

    /**
     * Replicates IMDB data sets and files that are not declared as a service dependency but needs
     * to be present before sipXconfig attempts to restart the services.
     */
    public void initLocations();

    public void markServiceForRestart(SipxService service);

    void markServiceForRestart(Location location, Collection< ? extends SipxService> services);

    void markServiceForRestart(Collection< ? extends SipxService> services);
}
