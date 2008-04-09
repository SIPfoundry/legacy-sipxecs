/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.site.sbc;

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDescriptor;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.ProfileManager;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListSbcDevices extends BasePage {
    public static final String PAGE = "sbc/ListSbcDevices";

    @InjectObject(value = "spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject(value = "spring:sbcProfileManager")
    public abstract ProfileManager getSbcProfileManager();

    @InjectObject(value = "spring:sbcModelSource")
    public abstract ModelSource<SbcDescriptor> getSbcDeviceModelSource();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract void setGenerateProfileIds(Collection<Integer> ids);

    public abstract Collection<Integer> getSbcsToDelete();

    public abstract Collection<Integer> getSbcsToPropagate();

    public abstract SbcDescriptor getSbcDescriptor();

    public abstract SbcDevice getSbc();

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }

    /**
     * When user clicks on link to edit a gateway
     */
    public IPage formSubmit(IRequestCycle cycle) {
        Collection selectedRows = getSbcsToDelete();
        if (selectedRows != null) {
            getSbcDeviceManager().deleteSbcDevices(selectedRows);
        }
        selectedRows = getSbcsToPropagate();
        if (selectedRows != null) {
            setGenerateProfileIds(selectedRows);
        }
        SbcDescriptor model = getSbcDescriptor();
        if (model != null) {
            return EditSbcDevice.getAddPage(cycle, model, this);
        }
        return null;
    }

    public void propagateAll() {
        Collection gatewayIds = getSbcDeviceManager().getAllSbcDeviceIds();
        setGenerateProfileIds(gatewayIds);
    }

    public IPage editSbc(IRequestCycle cycle, Integer sbcId) {
        return EditSbcDevice.getEditPage(cycle, sbcId, this);
    }

    public IPropertySelectionModel getSbcDescriptorSelectionModel() {
        DeviceDescriptorSelectionModel model = new DeviceDescriptorSelectionModel();
        model.setModelSource(getSbcDeviceModelSource());
        model.setExtraLabel(getMessages().getMessage("prompt.addNew"));
        model.setExtraOption(null);
        return model;
    }

    public static class DeviceDescriptorSelectionModel extends ExtraOptionModelDecorator {
        public void setModelSource(ModelSource modelSource) {
            ObjectSelectionModel model = new ObjectSelectionModel();
            model.setCollection(modelSource.getModels());
            model.setLabelExpression("label");
            setModel(model);
        }
    }
}
