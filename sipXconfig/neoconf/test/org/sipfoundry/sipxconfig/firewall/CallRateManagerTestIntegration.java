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

import java.util.ArrayList;
import java.util.List;

import org.mozilla.javascript.edu.emory.mathcs.backport.java.util.Collections;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.firewall.CallRateLimit.CallRateInterval;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class CallRateManagerTestIntegration extends IntegrationTestCase {
    private CallRateManager m_callRateManager;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    public void testSaveCallRateRules() {
        CallRateLimit limit = new CallRateLimit();
        limit.setSipMethodId(CallRateLimit.SipMethod.INVITE);
        limit.setRate(5);
        limit.setIntervalId(CallRateInterval.minute);
        CallRateRule rule = new CallRateRule();
        rule.setName("test");
        rule.setDescription("description");
        rule.setStartIp("zzzz");
        try {
            m_callRateManager.saveCallRateRule(rule);
            fail();
        } catch (UserException ex) {
            assertEquals("&msg.invalidcidr", ex.getMessage());
        }
        rule.setStartIp("192.168.0.1");
        List<CallRateLimit> limits = new ArrayList<CallRateLimit>();
        limits.add(limit);
        rule.setCallRateLimits(limits);
        m_callRateManager.saveCallRateRule(rule);
        assertEquals(1, m_callRateManager.getCallRateRules().size());
        CallRateRule rule1 = new CallRateRule();
        rule1.setName("test1");
        rule1.setDescription("description");
        rule1.setStartIp("192.168.0.1/32");
        m_callRateManager.saveCallRateRule(rule1);
        CallRateRule rule2 = new CallRateRule();
        rule2.setName("test2");
        rule2.setDescription("description");
        rule2.setStartIp("192.168.0.1/32");
        rule2.setEndIp("192.168.0.80");
        try {
            m_callRateManager.saveCallRateRule(rule2);
            fail();
        } catch (UserException ex) {
            assertEquals("&msg.invalidiprange", ex.getMessage());
        }
        rule2.setStartIp("192.168.0.1");
        rule2.setEndIp("192.168.0.80");
        m_callRateManager.saveCallRateRule(rule2);
        assertEquals(2, rule2.getPosition());
    }

    public void testDeleteCallRateRules() {
        CallRateRule rule = new CallRateRule();
        rule.setName("test");
        rule.setDescription("description");
        rule.setStartIp("10.1.1.1");
        m_callRateManager.saveCallRateRule(rule);
        assertEquals(1, m_callRateManager.getCallRateRules().size());
        m_callRateManager.deleteCallRateRules(Collections.singleton(rule));
        assertEquals(0, m_callRateManager.getCallRateRules().size());
    }

    public void setCallRateManager(CallRateManager callRateManager) {
        m_callRateManager = callRateManager;
    }
}
