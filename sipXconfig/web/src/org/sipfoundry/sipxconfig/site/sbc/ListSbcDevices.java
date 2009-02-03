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
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDescriptor;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.device.RestartManager;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListSbcDevices extends BasePage {
    public static final String PAGE = "sbc/ListSbcDevices";

    @InjectObject(value = "spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject(value = "spring:sbcProfileManager")
    public abstract ProfileManager getProfileManager();

    @InjectObject(value = "spring:sbcRestartManager")
    public abstract RestartManager getRestartManager();

    @InjectObject(value = "spring:sbcModelSource")
    public abstract ModelSource<SbcDescriptor> getSbcDeviceModelSource();

    @InjectObject("spring:dialPlanActivationManager")
    public abstract DialPlanActivationManager getDialPlanActivationManager();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract void setGenerateProfileIds(Collection<Integer> ids);

    public abstract SbcDescriptor getSbcDescriptor();

    public abstract void setSbcDescriptor(SbcDescriptor sbcDescriptor);

    public abstract SbcDevice getSbc();

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }

    /**
     * When user clicks on link to add an SBC
     */
    public IPage formSubmit(IRequestCycle cycle) {
        IValidationDelegate validator = TapestryUtils.getValidator(getPage());
        SbcDescriptor model = getSbcDescriptor();
        if (model == null) {
            return null;
        }

        // we have to try/catch the possible UserException since we need to reset
        // SBCs's models combobox selected value when it is thrown
        try {
            getSbcDeviceManager().checkForNewSbcDeviceCreation(model);
        } catch (UserException ex) {
            validator.record(new ValidatorException(ex.format(getMessages().getMessage(ex.getMessage()))));
            // reset SBCs's models combobox selected value
            setSbcDescriptor(null);
            return null;
        }
        return EditSbcDevice.getAddPage(cycle, model, this);
    }

    public void delete() {
        Collection<Integer> ids = getSelections().getAllSelected();
        if (!ids.isEmpty()) {
            getSbcDeviceManager().deleteSbcDevices(ids);
            getDialPlanActivationManager().replicateDialPlan(true); // restartSBCDevices == true
        }
    }

    public void propagate() {
        Collection<Integer> ids = getSelections().getAllSelected();
        if (!ids.isEmpty()) {
            setGenerateProfileIds(ids);
        }
    }

    public void propagateAll() {
        Collection ids = getSbcDeviceManager().getAllSbcDeviceIds();
        if (!ids.isEmpty()) {
            setGenerateProfileIds(ids);
        }
    }

    public void restart() {
        Collection<Integer> ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }
        getRestartManager().restart(ids, null);
        String msg = getMessages().format("msg.success.restart", Integer.toString(ids.size()));
        TapestryUtils.recordSuccess(this, msg);
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
