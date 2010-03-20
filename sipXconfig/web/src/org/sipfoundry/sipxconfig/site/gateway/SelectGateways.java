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
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.site.dialplan.EditDialRule;

/**
 * List all the gateways, allow adding and deleting gateways
 */
public abstract class SelectGateways extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "gateway/SelectGateways";

    // virtual properties
    public abstract DialPlanContext getDialPlanContext();

    public abstract GatewayContext getGatewayContext();

    public abstract Integer getRuleId();

    public abstract void setRuleId(Integer id);

    public abstract Collection getSelectedRows();

    public abstract void setGateways(Collection gateways);

    public abstract Collection getGateways();

    public abstract String getNextPage();

    public abstract void setNextPage(String nextPage);

    public void pageBeginRender(PageEvent event_) {
        Collection gateways = getGateways();
        if (null == gateways) {
            gateways = getGatewayContext().getAvailableGateways(getRuleId());
            setGateways(gateways);
        }
    }

    public IPage formSubmit(IRequestCycle cycle) {
        Collection selectedRows = getSelectedRows();
        if (selectedRows != null) {
            selectGateways(selectedRows);
        }
        EditDialRule editPage = (EditDialRule) cycle.getPage(getNextPage());
        editPage.setRuleId(getRuleId());
        return editPage;
    }

    /**
     * Adds/removes gateways from dial plan
     *
     * @param gatewayIds list of gateway ids to be added to the dial plan
     */
    void selectGateways(Collection gatewayIds) {
        DialPlanContext manager = getDialPlanContext();
        Integer ruleId = getRuleId();
        DialingRule rule = manager.getRule(ruleId);
        if (null == rule) {
            return;
        }
        List gateways = getGatewayContext().getGatewayByIds(gatewayIds);
        for (Iterator i = gateways.iterator(); i.hasNext();) {
            Gateway gateway = (Gateway) i.next();
            rule.addGateway(gateway);
        }
        manager.storeRule(rule);
    }
}
