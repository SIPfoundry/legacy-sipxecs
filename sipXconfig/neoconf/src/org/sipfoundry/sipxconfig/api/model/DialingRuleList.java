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
package org.sipfoundry.sipxconfig.api.model;

import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.sipxconfig.dialplan.DialingRule;

@XmlRootElement(name = "Rules")
public class DialingRuleList {
    private List<DialingRuleBean> m_rules;

    public void setRules(List<DialingRuleBean> rules) {
        m_rules = rules;
    }

    public static DialingRuleList convertDialingRuleList(List<DialingRule > rules) {
        List<DialingRuleBean> rulesList = new ArrayList<DialingRuleBean>();
        for (DialingRule rule : rules) {
            rulesList.add(DialingRuleBean.convertDialingRule(rule));
        }
        DialingRuleList list = new DialingRuleList();
        list.setRules(rulesList);
        return list;
    }

    @XmlElement(name = "Rule")
    public List<DialingRuleBean> getRules() {
        if (m_rules == null) {
            m_rules = new ArrayList<DialingRuleBean>();
        }
        return m_rules;
    }
}
