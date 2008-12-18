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
import java.util.List;
import java.util.Set;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class EditLocationPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/EditLocationPage";

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    @InjectObject("spring:acdContext")
    public abstract AcdContext getAcdContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getLocationId();

    public abstract void setLocationId(Integer locationId);

    public abstract void setLocationBean(Location location);

    public abstract Location getLocationBean();

    public abstract Collection<String> getAvailableTabNames();

    public abstract void setAvailableTabNames(Collection<String> tabNames);

    public abstract SipxServiceBundle getSelectedBundle();

    public abstract void setSelectedBundle(SipxServiceBundle bundle);

    @Persist
    @InitialValue("literal:listServices")
    public abstract String getTab();

    public void pageBeginRender(PageEvent event) {
        if (getLocationId() != null) {
            Location location = getLocationsManager().getLocation(getLocationId());
            setLocationBean(location);
        }

        setAvailableTabNames(Arrays.asList("configureLocation", "listServices"));
        setSelectedBundle(null);
    }

    public void saveLocation() {
        if (TapestryUtils.isValid(this)) {
            getLocationsManager().storeLocation(getLocationBean());
        }
    }

    public void formSubmit() {
        SipxServiceBundle bundle = getSelectedBundle();
        if (bundle == null) {
            return;
        }

        List<SipxService> services = getSipxServiceManager().getBundles().get(bundle);
        Location location = getLocationBean();
        location.addServices(services);
        getLocationsManager().storeLocation(getLocationBean());

        getSipxProcessContext().manageServices(getLocationBean(), services, Command.START);

        for (SipxService sipxService : services) {
            // FIXME: only works in UI - better publish an event that the new service has been
            // added
            if (sipxService instanceof SipxAcdService) {
                getAcdContext().addNewServer(getLocationBean());
            }
        }

        ServicesTable servicesTable = (ServicesTable) getComponent("servicesTable");
        servicesTable.refresh();
    }

    public IPropertySelectionModel getBundleSelectionModel() {
        Set<SipxServiceBundle> bundles = getSipxServiceManager().getBundles().keySet();

        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(bundles);
        model.setLabelExpression("name");

        LocalizedOptionModelDecorator decorator = new LocalizedOptionModelDecorator();
        decorator.setMessages(getMessages());
        decorator.setResourcePrefix("bundle.");
        decorator.setModel(model);

        ExtraOptionModelDecorator decoratedModel = new ExtraOptionModelDecorator();
        decoratedModel.setExtraLabel(getMessages().getMessage("prompt.addNewBundle"));
        decoratedModel.setExtraOption(null);
        return decoratedModel.decorate(decorator);
    }
}
