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
    SipxService getServiceByBeanId(String beanId);

    SipxService getServiceByName(String name);

    Collection<SipxService> getAllServices();

    void storeService(SipxService service);

    void replicateServiceConfig(SipxService service);

    Map<SipxServiceBundle, List<SipxService>> getBundles();

    List<SipxService> getRestartable();
}
