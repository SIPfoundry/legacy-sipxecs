/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.service;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.service.ServiceDescriptor;
import org.sipfoundry.sipxconfig.service.ServiceManager;
import org.sipfoundry.sipxconfig.service.UnmanagedService;


public abstract class UnmanagedServicePage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "service/UnmanagedServicePage";

    @InjectObject(value = "spring:serviceManager")
    public abstract ServiceManager getServiceManager();

    @Bean()
    public abstract SipxValidationDelegate getValidator();

    public abstract UnmanagedService getService();
    public abstract void setService(UnmanagedService service);

    @Persist()
    public abstract Integer getServiceId();
    public abstract void setServiceId(Integer serviceId);

    @Persist()
    public abstract ServiceDescriptor getServiceDescriptor();

    public abstract void setServiceDescriptor(ServiceDescriptor descriptor);

    public void newService(ServiceDescriptor descriptor) {
        setServiceId(null);
        setServiceDescriptor(descriptor);
    }

    public void editService(Integer serviceId) {
        setServiceId(serviceId);
    }

    public void apply() {
        if (!getValidator().getHasErrors()) {
            getServiceManager().saveService(getService());
            setServiceId(getService().getId());
        }
    }

    public void pageBeginRender(PageEvent event) {
        UnmanagedService service = getService();
        if (service == null) {
            Integer id = getServiceId();
            if (id == null) {
                service = (UnmanagedService) getServiceManager().newService(getServiceDescriptor());
            } else {
                service =  (UnmanagedService) getServiceManager().loadService(id);
                setServiceDescriptor(service.getDescriptor());
            }
            setService(service);
        }
    }
}
