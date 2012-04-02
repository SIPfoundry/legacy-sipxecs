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
        db().queryForInt("select 1 from firewall_rule where prioritize = true and address_type = ?",
                rule.getAddressType().getId());
        
        List<EditableFirewallRule> saved = m_firewallManager.getEditableFirewallRules();
        assertFalse("Saved rule (not new)", saved.get(0).isNew());
    }
    
    public void testSaveServerGroup() {
        ServerGroup g = new ServerGroup();
        g.setName("test");
        String servers = "1.1.1.1/32";
        g.setServerList(servers);
        m_firewallManager.saveServerGroup(g);
        int id = db().queryForInt("select firewall_server_group_id from firewall_server_group where servers = ?", servers);
        g = m_firewallManager.getServerGroup(id);
        g.setName("test2");
        m_firewallManager.saveServerGroup(g);
        int id2 = db().queryForInt("select firewall_server_group_id from firewall_server_group where name = 'test2'");
        assertEquals(id, id2);
    }

    public void setFirewallManager(FirewallManager firewallManager) {
        m_firewallManager = firewallManager;
    }
}
