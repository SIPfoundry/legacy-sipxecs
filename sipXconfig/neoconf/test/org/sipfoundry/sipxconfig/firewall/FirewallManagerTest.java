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

import static org.junit.Assert.assertEquals;

import java.util.Arrays;
import java.util.List;

import org.junit.Test;
import org.sipfoundry.sipxconfig.address.AddressType;


public class FirewallManagerTest {
    
    @Test
    public void changed() {
        FirewallManagerImpl manager = new FirewallManagerImpl();
        AddressType t1 = new AddressType("test1");
        AddressType t2 = new AddressType("test2");
        DefaultFirewallRule r1 = new DefaultFirewallRule(t1, FirewallRule.SystemId.PUBLIC);
        DefaultFirewallRule r2 = new DefaultFirewallRule(t2, FirewallRule.SystemId.PUBLIC);
        List<EditableFirewallRule> rules = Arrays.asList(new EditableFirewallRule(r1), new EditableFirewallRule(r2));
        EditableFirewallRule[] none = manager.getChanged(rules);
        assertEquals(0, none.length);
        rules.get(0).setPriority(true);
        EditableFirewallRule[] one = manager.getChanged(rules);
        assertEquals(1, one.length);
        assertEquals(rules.get(0), one[0]);
    }
}
