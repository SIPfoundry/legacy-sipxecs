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

import org.sipfoundry.sipxconfig.common.DataObjectSource;



public interface ServiceManager extends DataObjectSource<ConfiguredService> {
    public static final String CONTEXT_ID = "serviceManager";
    
    public ConfiguredService newService(ServiceDescriptor descriptor);

    public ConfiguredService loadService(Integer serviceId);
    
    public void saveService(ConfiguredService service);

    public void deleteService(ConfiguredService service);
    
    public void deleteServices(Collection<Integer> serviceIds);

    public ConfiguredService getServiceByName(String name);

    public Collection<ConfiguredService> getServices();

    public Collection<ConfiguredService> getEnabledServicesByType(ServiceDescriptor descriptor);
    
    public void clear();
}
