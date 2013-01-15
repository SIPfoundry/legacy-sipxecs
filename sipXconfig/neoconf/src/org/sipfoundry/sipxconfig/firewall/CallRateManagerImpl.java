/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.firewall;

import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.util.IPAddressUtil;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.jdbc.core.JdbcTemplate;

public class CallRateManagerImpl extends SipxHibernateDaoSupport implements CallRateManager {
    private JdbcTemplate m_jdbc;

    @Override
    public List<CallRateRule> getCallRateRules() {
        return getHibernateTemplate().findByNamedQuery("orderedCallRates");
    }

    public Integer countCallRateRules() {
        return m_jdbc.queryForInt("select max(position) from call_rate_rule");
    }

    @Override
    public void saveCallRateRule(CallRateRule rate) {
        if (rate == null) {
            throw new UserException("&err.noCallRate");
        }

        String err = "&msg.invalidcidr";
        String startIp = rate.getStartIp();
        if (!IPAddressUtil.isLiteralIPAddress(startIp) && !IPAddressUtil.isLiteralIPSubnetAddress(startIp)) {
            throw new UserException(err, startIp);
        }
        String endIp = rate.getEndIp();
        if (StringUtils.isNotBlank(endIp)) {
            if (!IPAddressUtil.isLiteralIPAddress(endIp)) {
                throw new UserException(err, endIp);
            }

            if (IPAddressUtil.isLiteralIPSubnetAddress(startIp)) {
                throw new UserException("&msg.invalidiprange");
            }
        }
        if (rate.isNew()) {
            rate.setPosition(countCallRateRules() + 1);
        }
        getHibernateTemplate().saveOrUpdate(rate);
    }

    @Override
    public CallRateRule getCallRateRule(Integer id) {
        return getHibernateTemplate().load(CallRateRule.class, id);
    }

    @Override
    public void deleteCallRateRules(Collection<CallRateRule> rules) {
        if (rules == null || rules.isEmpty()) {
            return;
        }
        for (CallRateRule rule : rules) {
            getHibernateTemplate().delete(rule);
        }
    }

    @Override
    public void saveCallRateRules(List<CallRateRule> rules) {
        int position = 0;
        for (CallRateRule rule : rules) {
            position++;
            rule.setPosition(position);
            saveCallRateRule(rule);
        }
    }

    public void setJdbc(JdbcTemplate jdbc) {
        m_jdbc = jdbc;
    }

}
