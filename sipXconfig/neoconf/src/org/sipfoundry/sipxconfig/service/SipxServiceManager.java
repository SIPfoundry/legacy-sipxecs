/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Collection;
import java.util.List;
import java.util.Map;

public interface SipxServiceManager {
    static final String CONTEXT_BEAN_NAME = "sipxServiceManager";

    SipxService getServiceByName(String name);
    SipxService getServiceByBeanId(String beanId);
    Collection<SipxService> getServiceDefinitions();
    void storeService(SipxService service);
    void replicateServiceConfig(SipxService service);
    Map<SipxServiceBundle, List<SipxService>> getBundles();

    List<SipxService> getRestartable();

    /**
     * verifies if the service is installed on the given location
     * @param serviceBeanId - service bean id
     * @return true if the service is installed on the given location / false otherwise
     */
    boolean isServiceInstalled(Integer locationId, String serviceBeanId);
    /**
     * verifies if the service is installed on any location
     * @param serviceBeanId - service bean id
     * @return true if there is at least one location with this service installed / false otherwise
     */
    boolean isServiceInstalled(String serviceBeanId);
}
