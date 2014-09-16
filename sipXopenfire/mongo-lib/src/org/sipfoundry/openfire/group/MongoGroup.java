/**
 *
 *
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
package org.sipfoundry.openfire.group;

import java.util.Collection;

import org.jivesoftware.openfire.group.DefaultGroupPropertyMap;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.util.PersistableMap;
import org.xmpp.packet.JID;

/**
 * Was only used by {@link MongoGroupProvider}
 */
@Deprecated
public class MongoGroup extends Group {
    private PersistableMap<String, String> m_properties;

    public MongoGroup() {

    }

    public MongoGroup(String name, String description, Collection<JID> members, Collection<JID> administrators) {
        super(name, description, members, administrators);
    }

    @Override
    public PersistableMap<String, String> getProperties() {
        if (m_properties == null) {
            m_properties = new DefaultGroupPropertyMap<String, String>(this);
        }
        return m_properties;
    }

}
