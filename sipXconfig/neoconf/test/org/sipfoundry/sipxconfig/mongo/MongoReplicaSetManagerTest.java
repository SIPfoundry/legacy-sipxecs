/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import static org.junit.Assert.assertEquals;

import java.util.Collections;
import java.util.Set;

import org.junit.Test;

public class MongoReplicaSetManagerTest {
    
    @Test
    public void updateMembers() {
        MongoReplicaSetManager m = new MongoReplicaSetManager();
        Set<String> none = Collections.emptySet();
        Set<String> a = Collections.singleton("a");
        Set<String> b = Collections.singleton("b");
        assertEquals("[]", m.updateMembersAndArbitors(a, none, a, none).toString());
        assertEquals("[rs.add(\"a:27017\")]", m.updateMembersAndArbitors(a, none, none, none).toString());
        assertEquals("[rs.remove(\"a:27017\")]", m.updateMembersAndArbitors(none, none, a, none).toString());
        assertEquals("[rs.add(\"a:27017\"), rs.remove(\"b:27017\")]", m.updateMembersAndArbitors(a, none, b, none).toString());
    }

    @Test
    public void updateArbiters() {
        MongoReplicaSetManager m = new MongoReplicaSetManager();
        Set<String> none = Collections.emptySet();
        Set<String> a = Collections.singleton("a");
        Set<String> b = Collections.singleton("b");
        assertEquals("[]", m.updateMembersAndArbitors(none, a, none, a).toString());
        assertEquals("[rs.addArb(\"a:27018\")]", m.updateMembersAndArbitors(none, a, none, none).toString());
        assertEquals("[rs.remove(\"a:27018\")]", m.updateMembersAndArbitors(none, none, none, a).toString());
        assertEquals("[rs.addArb(\"a:27018\"), rs.remove(\"b:27018\")]", m.updateMembersAndArbitors(none, a, none, b).toString());
    }
}
