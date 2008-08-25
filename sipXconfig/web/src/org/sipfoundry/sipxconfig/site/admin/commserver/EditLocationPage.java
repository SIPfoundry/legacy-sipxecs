/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.Process;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class EditLocationPage extends PageWithCallback implements
        PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/EditLocationPage";

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject(value = "spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject(value = "spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getLocationId();

    public abstract void setLocationId(Integer locationId);

    public abstract void setLocationBean(Location location);

    public abstract Location getLocationBean();

    public abstract Collection<String> getAvailableTabNames();

    public abstract void setAvailableTabNames(Collection<String> tabNames);

    public abstract SipxService getSelectedSipxService();

    public abstract void setSelectedSipxService(SipxService sipxService);

    @Persist
    @InitialValue(value = "literal:configureLocation")
    public abstract String getTab();

    public void pageBeginRender(PageEvent event) {
        if (getLocationId() != null) {
            Location location = getLocationsManager().getLocation(getLocationId());
            setLocationBean(location);
        }

        setAvailableTabNames(Arrays.asList(new String[] {
            "configureLocation", "listServices"
        }));
        setSelectedSipxService(null);
    }

    public void saveLocation() {
        if (TapestryUtils.isValid(this)) {
            getLocationsManager().storeLocation(getLocationBean());
        }
    }

    public void formSubmit() {
        if (getSelectedSipxService() == null) {
            return;
        }

        SipxService newService = getSelectedSipxService();
        getLocationBean().addService(newService);
        getLocationsManager().storeLocation(getLocationBean());

        Process newProcess = getSipxProcessContext().getProcess(newService.getProcessName());
        getSipxProcessContext().manageServices(getLocationBean(),
                Collections.singletonList(newProcess), Command.START);

        ServicesTable servicesTable = (ServicesTable) getComponent("servicesTable");
        servicesTable.refresh();
    }

    public IPropertySelectionModel getSipxServiceSelectionModel() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(getSipxServiceManager().getAllServices());
        model.setLabelExpression("beanId");

        ExtraOptionModelDecorator decoratedModel = new ExtraOptionModelDecorator();
        decoratedModel.setExtraLabel(getMessages().getMessage("prompt.addNewSipxService"));
        decoratedModel.setExtraOption(null);
        return decoratedModel.decorate(model);
    }
}
