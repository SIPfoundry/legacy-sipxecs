/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
