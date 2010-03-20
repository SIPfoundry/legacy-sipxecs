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

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.service.ConfiguredService;
import org.sipfoundry.sipxconfig.service.ServiceDescriptor;
import org.sipfoundry.sipxconfig.service.ServiceManager;

public abstract class ListConfiguredServices extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "service/ListConfiguredServices";

    @InjectPage(value = UnmanagedServicePage.PAGE)
    public abstract UnmanagedServicePage getUnmanagedServicePage();

    @InjectObject(value = "spring:serviceManager")
    public abstract ServiceManager getServiceManager();

    @InjectObject(value = "spring:serviceDescriptorSource")
    public abstract ModelSource<ServiceDescriptor> getServiceDescriptorSource();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestryContext();

    @Bean()
    public abstract SipxValidationDelegate getValidator();

    @Bean()
    public abstract SelectMap getSelections();

    @InitialValue("createServiceSelectionModel()")
    public abstract IPropertySelectionModel getServiceSelectionModel();

    public abstract ConfiguredService getCurrentRow();

    public abstract void setConverter(IPrimaryKeyConverter converter);

    public abstract ServiceDescriptor getServiceDescriptor();

    public abstract Collection<Integer> getRowsToDelete();

    public void pageBeginRender(PageEvent event) {
        if (getRequestCycle().isRewinding()) {
            setConverter(new ObjectSourceDataSqueezer(getServiceManager(), ConfiguredService.class));
        }
    }

    public IPage formSubmit() {
        if (getServiceDescriptor() != null) {
            UnmanagedServicePage page = getUnmanagedServicePage();
            page.newService(getServiceDescriptor());
            page.setReturnPage(PAGE);
            return page;
        }
        Collection<Integer> toDelete = getRowsToDelete();
        if (toDelete != null) {
            getServiceManager().deleteServices(toDelete);
        }
        return this;
    }

    public IPage edit(Integer serviceId) {
        UnmanagedServicePage page = getUnmanagedServicePage();
        page.editService(serviceId);
        page.setReturnPage(PAGE);
        return page;
    }

    public IPropertySelectionModel createServiceSelectionModel() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(getServiceDescriptorSource().getModels());
        model.setLabelExpression("label");

        return getTapestryContext().addExtraOption(model, getMessages(), "label.addNewService");
    }
}
