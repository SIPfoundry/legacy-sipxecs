/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.site.gateway.EditGateway;

/**
 * GatewayTable
 */
public abstract class GatewayTable extends BaseComponent implements PageBeginRenderListener {
    public abstract Gateway getCurrentRow();

    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);

    @Asset("/images/gateway_shared.png")
    public abstract IAsset getSharedGwIcon();

    public void pageBeginRender(PageEvent event_) {
        SelectMap selections = getSelections();
        if (null == selections) {
            setSelections(new SelectMap());
        }
    }

    /**
     * When user clicks on link to edit a gateway
     */
    public IPage edit(IRequestCycle cycle, Integer id, Integer ruleId) {
        return EditGateway.getEditPage(cycle, id, getPage(), ruleId);
    }
}
