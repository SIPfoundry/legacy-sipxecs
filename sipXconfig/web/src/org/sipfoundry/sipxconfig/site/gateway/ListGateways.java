/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway;

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.Component;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.GatewayTable;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.device.RestartManager;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.site.sbc.ListSbcDevices.DeviceDescriptorSelectionModel;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends SipxBasePage {
    public static final String PAGE = "gateway/ListGateways";

    @InjectObject(value = "spring:gatewayContext")
    public abstract GatewayContext getGatewayContext();

    @InjectObject(value = "spring:gatewayProfileManager")
    public abstract ProfileManager getProfileManager();

    @InjectObject(value = "spring:gatewayRestartManager")
    public abstract RestartManager getRestartManager();

    @InjectObject(value = "spring:gatewayModelSource")
    public abstract ModelSource getModelSource();

    public abstract GatewayModel getGatewayModel();

    @Component(id = "gatewayTable", type = "gateway/GatewayTable")
    public abstract GatewayTable getGatewayTable();

    @Persist
    public abstract void setGenerateProfileIds(Collection<Integer> ids);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    /**
     * When user clicks on link to add a new gateway
     */
    public IPage formSubmit(IRequestCycle cycle) {
        GatewayModel model = getGatewayModel();
        if (model == null) {
            return null;
        }
        return EditGateway.getAddPage(cycle, model, this, null);
    }

    public void propagate() {
        Collection<Integer> ids = getGatewayTable().getSelections().getAllSelected();
        if (!ids.isEmpty()) {
            setGenerateProfileIds(ids);
        }
    }

    public void delete() {
        Collection<Integer> ids = getGatewayTable().getSelections().getAllSelected();
        if (!ids.isEmpty()) {
            getGatewayContext().deleteGateways(ids);
        }
    }

    public void restart() {
        Collection<Integer> ids = getGatewayTable().getSelections().getAllSelected();
        if (!ids.isEmpty()) {
            getRestartManager().restart(ids, null);
            String msg = getMessages().format("msg.success.restart", Integer.toString(ids.size()));
            TapestryUtils.recordSuccess(this, msg);
        }
    }

    public void propagateAll() {
        Collection gatewayIds = getGatewayContext().getAllGatewayIds();
        if (!gatewayIds.isEmpty()) {
            setGenerateProfileIds(gatewayIds);
        }
    }

    public IPropertySelectionModel getGatewaySelectionModel() {
        DeviceDescriptorSelectionModel model = new DeviceDescriptorSelectionModel();
        model.setModelSource(getModelSource());
        model.setExtraLabel(getMessages().getMessage("addNewGateway"));
        model.setExtraOption(null);
        return model;
    }
}
