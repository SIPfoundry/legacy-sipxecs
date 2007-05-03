/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxconfig.site.gateway.port;

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class GatewayPorts extends BaseComponent {
    @InjectPage(value = PortSettings.PAGE)
    public abstract PortSettings getPortSettingsPage();

    @InjectObject(value = "spring:gatewayContext")
    public abstract GatewayContext getGatewayContext();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestryContext();

    @Parameter
    public abstract Gateway getGateway();

    public abstract void setGateway(Gateway gateway);

    public abstract List<FxoPort> getPorts();

    public abstract void setPorts(List<FxoPort> ports);

    public abstract FxoPort getPort();

    public abstract void setPort(FxoPort port);

    @InitialValue(value = "new org.sipfoundry.sipxconfig.components.SelectMap()")
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selections);

    protected void prepareForRender(IRequestCycle cycle) {
        if (getPorts() != null) {
            return;
        }
        setPorts(getGateway().getPorts());
    }

    public IPage addPort() {
        Gateway gateway = getGateway();
        gateway.addPort(new FxoPort());
        getGatewayContext().storeGateway(gateway);
        int last = gateway.getPorts().size() - 1;
        FxoPort port = gateway.getPorts().get(last);

        return editPort(port.getId());
    }

    public IPage editPort(Integer portId) {
        PortSettings editPortPage = getPortSettingsPage();
        editPortPage.setPortId(portId);
        editPortPage.setReturnPage(getPage());
        return editPortPage;
    }

    public void deletePorts() {
        Collection allSelected = getSelections().getAllSelected();
        if (!allSelected.isEmpty()) {
            getGatewayContext().removePortsFromGateway(getGateway().getId(), allSelected);
            setPorts(null);
            // setting null resets page for fresh data
            setGateway(null);
        }
    }
}
