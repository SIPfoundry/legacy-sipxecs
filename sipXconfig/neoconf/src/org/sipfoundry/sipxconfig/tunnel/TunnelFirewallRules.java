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
package org.sipfoundry.sipxconfig.tunnel;

import static java.lang.String.format;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.firewall.CustomFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallTable;

/**
 * Create the firewall redirect rules based on the current tunnels to/from each server
 */
public class TunnelFirewallRules {
    private String m_rule = "-A OUTPUT -o $(sipx.iface) -p %s -d %s "
            + "--dport %d -j DNAT --to-destination 127.0.0.1:%d";

    public Collection<CustomFirewallRule> build(Collection<RemoteOutgoingTunnel> tunnels) {
        List<CustomFirewallRule> rules = new ArrayList<CustomFirewallRule>();
        for (RemoteOutgoingTunnel out : tunnels) {
            AllowedIncomingTunnel in = out.getIncomingTunnel();
            String rule = format(m_rule, out.getProtocol(), out.getRemoteMachineAddress(), in.getLocalhostPort(),
                    out.getLocalhostPort());
            rules.add(new CustomFirewallRule(FirewallTable.nat, rule));
        }
        return rules;
    }
}
