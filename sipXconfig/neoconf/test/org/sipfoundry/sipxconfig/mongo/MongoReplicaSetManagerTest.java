/**
 *
 *
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
        assertEquals("[rs.add(\"a\")]", m.updateMembersAndArbitors(none, none, a, none).toString());
        assertEquals("[rs.remove(\"a\")]", m.updateMembersAndArbitors(a, none, none, none).toString());
        assertEquals("[rs.add(\"b\"), rs.remove(\"a\")]", m.updateMembersAndArbitors(a, none, b, none).toString());
    }

    @Test
    public void updateArbiters() {
        MongoReplicaSetManager m = new MongoReplicaSetManager();
        Set<String> none = Collections.emptySet();
        Set<String> a = Collections.singleton("a");
        Set<String> b = Collections.singleton("b");
        assertEquals("[]", m.updateMembersAndArbitors(none, a, none, a).toString());
        assertEquals("[rs.addArb(\"a\")]", m.updateMembersAndArbitors(none, none, none, a).toString());
        assertEquals("[rs.remove(\"a\")]", m.updateMembersAndArbitors(none, a, none, none).toString());
        assertEquals("[rs.addArb(\"b\"), rs.remove(\"a\")]", m.updateMembersAndArbitors(none, a, none, b).toString());
    }
}
