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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.site.dialplan.SelectRuleType;

public abstract class GatewayNavigation extends BaseComponent implements PageBeginRenderListener {

    @Parameter(required = true)
    public abstract void setGateway(Gateway gateway);

    public abstract Gateway getGateway();

    public abstract void setDialingRule(DialingRule rule);

    public abstract Integer getDialingRuleId();

    @Parameter()
    public abstract void setDialingRuleId(Integer ruleId);

    @InjectObject("spring:dialPlanContext")
    public abstract DialPlanContext getDialPlanContext();

    public IPage editGateway(Integer gatewayId) {
        IPage editGateways = getPage().getRequestCycle().getPage(ListGateways.PAGE);
        return EditGateway.getEditPage(getPage().getRequestCycle(), gatewayId, editGateways, getDialingRuleId());
    }

    public IPage editRule(Integer ruleId) {
        DialingRule rule = getDialPlanContext().getRule(ruleId);
        return SelectRuleType.getEditDialRulePage(getPage().getRequestCycle(), rule.getType(), ruleId);
    }

    public void pageBeginRender(PageEvent event) {
        Integer ruleId = getDialingRuleId();
        if (ruleId != null) {
            setDialingRule(getDialPlanContext().getRule(ruleId));
        }
    }
}
