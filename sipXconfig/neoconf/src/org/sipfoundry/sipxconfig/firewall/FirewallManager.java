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

import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface FirewallManager {
    public static final GlobalFeature FEATURE = new GlobalFeature("firewall");

    public final class Util {
        private Util() {
        }

        public static <T extends FirewallRule> Map<String, T> defaultsByAddressTypeId(Collection<T> l) {
            Map<String, T> ndx = new HashMap<String, T>();
            for (T i : l) {
                ndx.put(i.getAddressType().getId(), i);
            }
            return ndx;
        }

        public static Map<Integer, ServerGroup> groupsById(Collection<ServerGroup> l) {
            Map<Integer, ServerGroup> ndx = new HashMap<Integer, ServerGroup>();
            for (ServerGroup i : l) {
                ndx.put(i.getId(), i);
            }
            return ndx;
        }
    }

    public ServerGroup getServerGroup(Integer groupId);

    public void saveServerGroup(ServerGroup serverGroup);

    public void deleteServerGroup(ServerGroup serverGroup);

    public FirewallSettings getSettings();

    public void saveSettings(FirewallSettings settings);

    public List<ServerGroup> getServerGroups();

    public List<DefaultFirewallRule> getDefaultFirewallRules();

    public List<EditableFirewallRule> getEditableFirewallRules();

    public void saveRules(List<EditableFirewallRule> rules);

    public List<FirewallRule> getFirewallRules();

    public List<CustomFirewallRule> getCustomRules(Location location, Map<Object, Object> requestData);

}
