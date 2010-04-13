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
import org.apache.tapestry.IActionListener;
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
    private int m_index;

    @InjectPage(value = PortSettings.PAGE)
    public abstract PortSettings getPortSettingsPage();

    @InjectObject(value = "spring:gatewayContext")
    public abstract GatewayContext getGatewayContext();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestryContext();

    @Parameter
    public abstract Gateway getGateway();

    @Parameter(required = true)
    public abstract IActionListener getEditPortListener();

    @Parameter(required = true)
    public abstract IActionListener getAddPortListener();

    public abstract void setGateway(Gateway gateway);

    public abstract List<FxoPort> getPorts();

    public abstract void setPorts(List<FxoPort> ports);

    public abstract FxoPort getPort();

    public abstract void setPort(FxoPort port);

    @InitialValue(value = "new org.sipfoundry.sipxconfig.components.SelectMap()")
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selections);

    public int getIndex() {
        return ++m_index;
    }

    public void resetIndex() {
        m_index = 0;
    }

    protected void prepareForRender(IRequestCycle cycle) {
        resetIndex();
        if (getPorts() != null) {
            return;
        }
        setPorts(getGateway().getPorts());
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
