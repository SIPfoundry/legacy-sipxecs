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

import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.service.SipxService;

public interface SipxProcessContext {
    enum Command {
        START, STOP, RESTART;
    }

    /**
     * Return an array containing a ServiceStatus entry for each process on the first server
     * machine. This is a first step towards providing status for all server machines.
     *
     * @param onlyActiveServices If true, only return status information for the services that the
     *        location parameter lists in its services list. If false, return all service status
     *        information available.
     */
    ServiceStatus[] getStatus(Location location, boolean onlyActiveServices);

    /**
     * Get status for a particular service on a given location
     */
    ServiceStatus.Status getStatus(Location location, SipxService service);

    /**
     * Return a list of status messages for a given service on a given server.
     */
    List<String> getStatusMessages(Location location, SipxService service);

    /**
     * Apply the specified command to the named services. This method handles only commands that
     * don't need output, which excludes the "status" command.
     *
     * @param services list of services that will receive the command
     * @param command command to send
     * @param location information about the host on which services are running
     */
    void manageServices(Location location, Collection< ? extends SipxService> services, Command command);

    /**
     * Perform operation on specific location services. Make sure that services from location
     * that contains the configuration service are operated last. Otherwise some services may
     * not be managed anymore because the configuration service (Jetty Context) may be down and cannot launch
     * any other call
     *
     * @param servicesMap a map that defines which services on which location has to be managed
     * @param command
     */
    void manageServices(Map<Location, List<SipxService>> servicesMap, Command command);

    /**
     * Retrieve the list of the services that should be started and stopped on the location to
     * make it to conform to bundle list.
     */
    LocationStatus getLocationStatus(Location location);

    void markServicesForRestart(Location location, Collection< ? extends SipxService> processes);

    void markServicesForRestart(Collection< ? extends SipxService> processes);

    /**
     * Restart the services that are marked for restart on a specified location.
     */
    void restartMarkedServices(Location location);

    boolean needsRestart();

    boolean needsRestart(Location location, SipxService service);

    Collection<RestartNeededService> getRestartNeededServices();

    /**
     * Used in test only: clears the need for restart status
     */
    void clear();

    /**
     * A special version of markServiceForRestart. It does all what markServicesForRestart does
     * *and* also Forces SipxProxyContext to activate the dial plan before restarting any of the
     * services listed here.
     *
     * @param the beanId beanId of the service, restart of which should cause dial plan
     *        replication
     */
    void markDialPlanRelatedServicesForRestart(String... serviceBeansIds);

    void unmarkServicesToRestart(Collection<RestartNeededService> services);
}
