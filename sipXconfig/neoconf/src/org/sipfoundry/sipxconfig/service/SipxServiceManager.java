/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public interface SipxServiceManager {
    String CONTEXT_BEAN_NAME = "sipxServiceManager";

    SipxService getServiceByName(String name);

    SipxService getServiceByBeanId(String beanId);

    Collection<SipxService> getServiceDefinitions();

    SipxServiceBundle getBundleByName(String name);

    Collection<SipxServiceBundle> getBundleDefinitions();

    void storeService(SipxService service);

    Map<SipxServiceBundle, List<SipxService>> getBundles();

    List<SipxService> getRestartable();

    /**
     * Verifies if the service is installed on the given location
     *
     * @param serviceBeanId - service bean id
     * @return true if the service is installed on the given location / false otherwise
     */
    boolean isServiceInstalled(Integer locationId, String serviceBeanId);

    /**
     * verifies if the service is installed on any location
     *
     * @param serviceBeanId - service bean id
     * @return true if there is at least one location with this service installed / false
     *         otherwise
     */
    boolean isServiceInstalled(String serviceBeanId);

    /**
     * Retrieves the list of bundles installed on a specific location.
     *
     * @param locationBean
     * @return list of installed bundles
     */
    List<SipxServiceBundle> getBundlesForLocation(Location location);

    /**
     * Updates location services based on the bundles definitions.
     *
     * @param location affected location
     * @param bundles list of bundles that should be installed on this location
     */
    void setBundlesForLocation(Location location, List<SipxServiceBundle> bundles);

    /**
     * Create collection of the services that belong to the specific subset of bundles
     */
    Collection<SipxService> getServiceDefinitions(final Collection<SipxServiceBundle> bundles);

    Object getServiceParam(String paramName);
}
