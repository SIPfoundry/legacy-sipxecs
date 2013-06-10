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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class FirewallConfigTest {
    private FirewallConfig m_config;
    private FirewallSettings m_settings;
    private StringWriter m_actual;        
    
    @Before
    public void setUp() {
        m_config = new FirewallConfig();
        m_settings = new FirewallSettings();
        m_settings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_actual = new StringWriter();        
    }
    
    @Test
    public void cfdat() throws IOException {
        List<String> mods = Arrays.asList("mod1", "mod2");
        m_config.writeCfdat(m_actual, true, m_settings.getSystemSettings(), mods);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-cfdat"));
        assertEquals(expected, m_actual.toString());
    }

    @Test
    public void sysctl() throws IOException {
        m_config.writeSysctl(m_actual, m_settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sysctl.part"));
        assertEquals(expected, m_actual.toString());
    }

    @Test
    public void iptables() throws IOException {        
        List<FirewallRule> rules = new ArrayList<FirewallRule>();
        AddressType t1 = new AddressType("t1");
        AddressType t2 = new AddressType("t2");
        AddressType t3 = new AddressType("t3");
        rules.add(new DefaultFirewallRule(t1, FirewallRule.SystemId.CLUSTER));
        rules.add(new DefaultFirewallRule(t2, FirewallRule.SystemId.PUBLIC));
        Location l1 = new Location("one", "1.1.1.1");
        Location l2 = new Location("two", "2.2.2.2");
        List<Location> cluster = Arrays.asList(l1, l2);
        
        ServerGroup g1 = new ServerGroup("ClassA", "192.168.1.0/8 192.168.2.1/32");
        ServerGroup g2 = new ServerGroup("ClassB", "192.168.0.0/16");
        List<ServerGroup> groups = Arrays.asList(g1, g2);

        DefaultFirewallRule r3 = new DefaultFirewallRule(t3, FirewallRule.SystemId.PUBLIC);
        EditableFirewallRule editable = new EditableFirewallRule(r3);
        editable.setServerGroup(g1);
        rules.add(editable);

        AddressManager addressManager = createMock(AddressManager.class);
        addressManager.getAddresses(t1, l1);
        Address a1 = new Address(t1, l1.getAddress(), 100);
        Address a2 = new Address(t1, l1.getAddress(), 200);
        Address a3 = new Address(t2, l1.getAddress(), 300);
        Address a4 = new Address(t2, l1.getAddress(), 400);
        Address aIgnored = new Address(t1, l2.getAddress(), 400);
        expectLastCall().andReturn(Arrays.asList(a1, a2, aIgnored)).once();
        addressManager.getAddresses(t2, l1);
        expectLastCall().andReturn(Arrays.asList(a3, aIgnored)).once();
        addressManager.getAddresses(t3, l1);
        expectLastCall().andReturn(Arrays.asList(a4, aIgnored)).once();
        replay(addressManager);
        m_config.setAddressManager(addressManager);
        
        List<CustomFirewallRule> custom = Arrays.asList(
          new CustomFirewallRule(FirewallTable.nat, "nat 2"),
          new CustomFirewallRule(FirewallTable.nat, "nat 1"),
          new CustomFirewallRule(FirewallTable.mangle, "mangle 1")
        );

        CallRateRule rule = new CallRateRule();
        rule.setName("rule1");
        rule.setStartIp("192.168.0.1");
        CallRateLimit limit = new CallRateLimit();
        limit.setSipMethod("INVITE");
        limit.setRate(5);
        limit.setInterval("minute");
        List<CallRateLimit> limits = new ArrayList<CallRateLimit>();
        limits.add(limit);
        rule.setCallRateLimits(limits);
        CallRateRule rule1 = new CallRateRule();
        rule1.setName("rule2");
        rule1.setStartIp("192.168.0.2");
        rule1.setEndIp("192.168.0.4");
        rule1.setCallRateLimits(limits);
        List<CallRateRule> rateRules = new LinkedList<CallRateRule>();
        rateRules.add(rule);
        rateRules.add(rule1);
        m_config.writeIptables(m_actual, true, new HashSet<String>(), new HashSet<String>(), m_settings, rateRules, rules, custom, groups, cluster, l1);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-firewall.yaml"));
        assertEquals(expected, m_actual.toString());
        
        verify(addressManager);
    }
}
