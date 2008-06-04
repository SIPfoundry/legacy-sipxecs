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

public interface SipxServiceManager {
    public SipxService getServiceByBeanId(String beanId);
    public Collection<SipxService> getAllServices();
    public void storeService(SipxService service);
    public void replicateServiceConfig(SipxService service);
}
