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
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends BasePage {
    public static final String PAGE = "ListGateways";

    public abstract GatewayContext getGatewayContext();

    public abstract ProfileManager getGatewayProfileManager();

    public abstract Collection getGatewaysToDelete();

    public abstract Collection getGatewaysToPropagate();

    public abstract GatewayModel getGatewayModel();

    /**
     * When user clicks on link to edit a gateway
     */
    public IPage formSubmit(IRequestCycle cycle) {
        Collection selectedRows = getGatewaysToDelete();
        if (selectedRows != null) {
            getGatewayContext().deleteGateways(selectedRows);
        }
        selectedRows = getGatewaysToPropagate();
        if (selectedRows != null) {
            propagateGateways(selectedRows);
        }
        GatewayModel model = getGatewayModel();
        if (model != null) {
            return EditGateway.getAddPage(cycle, model, this, null);
        }
        return null;
    }

    public void propagateAllGateways() {
        Collection gatewayIds = getGatewayContext().getAllGatewayIds();
        propagateGateways(gatewayIds);
    }

    private void propagateGateways(Collection gatewayIds) {
        getGatewayProfileManager().generateProfiles(gatewayIds, true);
        String msg = getMessages().format("msg.success.profiles",
                Integer.toString(gatewayIds.size()));
        TapestryUtils.recordSuccess(this, msg);
    }
}
