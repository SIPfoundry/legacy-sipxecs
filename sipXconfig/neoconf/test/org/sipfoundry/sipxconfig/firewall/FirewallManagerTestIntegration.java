/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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

import java.util.List;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class FirewallManagerTestIntegration extends IntegrationTestCase {
    private FirewallManager m_firewallManager;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    public void testDefaultFirewallRules() {
        List<DefaultFirewallRule> rules = m_firewallManager.getDefaultFirewallRules();
        assertTrue(rules.size() > 0);
    }

    public void testEditableFirewallRules() {
        List<EditableFirewallRule> rules = m_firewallManager.getEditableFirewallRules();
        EditableFirewallRule rule = rules.get(0);
        boolean def = rule.isPriority();
        rule.setPriority(!def);
        m_firewallManager.saveRules(rules);
        db().execute("select 1 from firewall_rule where prioritize = true");
    }

    public void setFirewallManager(FirewallManager firewallManager) {
        m_firewallManager = firewallManager;
    }
}
