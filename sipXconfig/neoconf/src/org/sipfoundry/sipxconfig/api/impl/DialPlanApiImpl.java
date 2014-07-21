/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.api.impl;

import java.util.Collection;
import java.util.List;

import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.sipfoundry.sipxconfig.api.DialPlanApi;
import org.sipfoundry.sipxconfig.api.model.DialingRuleBean;
import org.sipfoundry.sipxconfig.api.model.DialingRuleList;
import org.sipfoundry.sipxconfig.api.model.NameList;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.dialplan.DialingRuleFactory;

public class DialPlanApiImpl implements DialPlanApi {
    private DialPlanContext m_dialPlanContext;
    private DialingRuleFactory m_dialingRuleFactory;

    @Override
    public Response getRawRules() {
        Collection<String> rawRules = m_dialingRuleFactory.getBeanIds();
        if (rawRules != null) {
            return Response.ok().entity(NameList.retrieveList(rawRules)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    public DialPlanContext getDialPlanContext() {
        return m_dialPlanContext;
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public DialingRuleFactory getDialingRuleFactory() {
        return m_dialingRuleFactory;
    }

    public void setDialingRuleFactory(DialingRuleFactory dialingRuleFactory) {
        m_dialingRuleFactory = dialingRuleFactory;
    }

    @Override
    public Response getRules() {
        List<DialingRule> rules = m_dialPlanContext.getRules();
        if (rules != null) {
            return Response.ok().entity(DialingRuleList.convertDialingRuleList(rules)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getRule(Integer ruleId) {
        DialingRule rule = m_dialPlanContext.getRule(ruleId);
        if (rule != null) {
            DialingRuleBean bean = DialingRuleBean.convertDialingRule(rule);
            return Response.ok().entity(bean).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }
}
