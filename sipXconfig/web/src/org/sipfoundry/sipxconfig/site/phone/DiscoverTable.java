/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.VendorFilteredDeviceSelectionModel;
import org.sipfoundry.sipxconfig.device.DiscoveredDevice;
import org.sipfoundry.sipxconfig.device.FilteredModelSource;

public abstract class DiscoverTable extends BaseComponent {

    public abstract SelectMap getSelections();

    public abstract Object getSource();

    @InjectObject(value = "spring:phoneModelSource")
    public abstract FilteredModelSource getPhoneModelSource();

    @InjectObject(value = "spring:gatewayModelSource")
    public abstract FilteredModelSource getGatewayModelSource();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestryContext();

    public abstract void setTargets(List<DiscoveredDevice> targets);

    public abstract List<DiscoveredDevice> getTargets();

    public IPropertySelectionModel getDeviceSelectionModel(String vendor) {
        VendorFilteredDeviceSelectionModel model = new VendorFilteredDeviceSelectionModel();
        model.setVendorFilter(vendor);
        model.setLabelExpression("label");
        model.setValueExpression("");

        Collection phoneCollection = getPhoneModelSource().getModels();
        Collection gatewayCollection = getGatewayModelSource().getModels();
        Collection deviceCollection = new ArrayList();
        deviceCollection.addAll(phoneCollection);
        deviceCollection.addAll(gatewayCollection);
        model.setCollection(deviceCollection);

        return getTapestryContext().instructUserToSelect(model, getMessages());
    }
}
