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

import org.apache.commons.collections.Predicate;

public class CustomFirewallRule {
    private FirewallTable m_table;
    private String m_rule;

    public CustomFirewallRule() {
    }

    public CustomFirewallRule(FirewallTable table, String rule) {
        m_table = table;
        m_rule = rule;
    }

    public static Predicate byTable(final FirewallTable t) {
        return new Predicate() {
            public boolean evaluate(Object arg0) {
                return t == ((CustomFirewallRule) arg0).m_table;
            }
        };
    }

    public FirewallTable getTable() {
        return m_table;
    }

    public void setTable(FirewallTable table) {
        m_table = table;
    }

    public String getRule() {
        return m_rule;
    }

    public void setRule(String rule) {
        m_rule = rule;
    }

    public String toString() {
        return m_rule;
    }
}
