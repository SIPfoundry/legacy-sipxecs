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
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class ListGateways extends BasePage {
    public static final String PAGE = "gateway/ListGateways";

    public abstract GatewayContext getGatewayContext();

    public abstract ProfileManager getGatewayProfileManager();

    public abstract Collection getGatewaysToDelete();

    public abstract Collection getGatewaysToPropagate();

    public abstract GatewayModel getGatewayModel();
    
    @Persist
    public abstract void setGenerateProfileIds(Collection<Integer> ids);

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
            setGenerateProfileIds(selectedRows);
        }
        GatewayModel model = getGatewayModel();
        if (model != null) {
            return EditGateway.getAddPage(cycle, model, this, null);
        }
        return null;
    }

    public void propagateAllGateways() {
        Collection gatewayIds = getGatewayContext().getAllGatewayIds();
        setGenerateProfileIds(gatewayIds);
    }
}
