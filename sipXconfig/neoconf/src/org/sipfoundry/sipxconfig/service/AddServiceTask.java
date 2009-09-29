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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.SystemTaskEntryPoint;
import org.sipfoundry.sipxconfig.device.ModelSource;

/**
 * Add a new service to the system.  If the service with same name and type exists
 * this will update it.  If a service exists w/different type, it will error.
 */
public class AddServiceTask implements SystemTaskEntryPoint {
    private static final Log LOG = LogFactory.getLog(AddServiceTask.class);
    private ServiceManager m_serviceManager;
    private ModelSource<ServiceDescriptor> m_servicesSource;

    public void runSystemTask(String[] args) {
        String serviceName = assertArgument(args, 1, "-Dservice.name");
        String serviceId = assertArgument(args, 2, "-Dservice.id");
        String serviceAddress = assertArgument(args, 3, "-Dservice.address");
        addService(serviceName, serviceId, serviceAddress);
    }

    void addService(String serviceName, String serviceId, String serviceAddress) {
        ConfiguredService service =  m_serviceManager.getServiceByName(serviceName);
        if (service != null) {
            if (!service.getDescriptorId().equals(serviceId)) {
                String msg = String.format("There is already a service named %s with type %s", serviceName, serviceId);
                throw new IllegalArgumentException(msg);
            }
        } else {
            ServiceDescriptor descriptor = m_servicesSource.getModel(serviceId);
            service = m_serviceManager.newService(descriptor);
            service.setName(serviceName);
        }

        service.setAddress(serviceAddress);
        m_serviceManager.saveService(service);
        LOG.info(String.format("service %s added", serviceName));
    }

    static String assertArgument(String[] args, int index, String label) {
        if (args == null || args.length <= index) {
            throw new IllegalArgumentException(String.format("Argument '%s' is required", label));
        }
        return args[index];
    }

    public void setServiceManager(ServiceManager serverManager) {
        m_serviceManager = serverManager;
    }

    public void setServicesSource(ModelSource<ServiceDescriptor> servicesSource) {
        m_servicesSource = servicesSource;
    }
}
