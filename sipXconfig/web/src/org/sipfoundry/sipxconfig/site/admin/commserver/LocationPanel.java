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

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class LocationPanel extends BaseComponent {
    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Parameter
    public abstract ICallback getCallback();

    @Parameter(required = true)
    public abstract Location getLocationBean();

    public abstract void setLocationBean(Location location);

    public abstract List<SipxServiceBundle> getBundles();

    public abstract void setBundles(List<SipxServiceBundle> bundles);

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        if (getBundles() == null) {
            Location location = getLocationBean();
            List<SipxServiceBundle> installedBundles = getSipxServiceManager().getBundlesForLocation(location);
            setBundles(installedBundles);
        }
    }

    public IPropertySelectionModel getBundlesModel() {
        Collection<SipxServiceBundle> bundles = getSipxServiceManager().getBundleDefinitions();

        ObjectSelectionModel nakedModel = new ObjectSelectionModel();
        nakedModel.setCollection(bundles);
        nakedModel.setLabelExpression("name");

        LocalizedOptionModelDecorator model = new LocalizedOptionModelDecorator();
        model.setMessages(getMessages());
        model.setResourcePrefix("bundle.");
        model.setModel(nakedModel);

        return model;
    }

    public void saveLocation() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        Location location = getLocationBean();
        List<SipxServiceBundle> bundles = getBundles();
        getSipxServiceManager().setBundlesForLocation(location, bundles);

        getLocationsManager().storeLocation(location);
    }
}
