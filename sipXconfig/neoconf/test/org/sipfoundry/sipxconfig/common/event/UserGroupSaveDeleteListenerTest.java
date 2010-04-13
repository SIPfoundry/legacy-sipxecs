/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.common.event;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Group;

public class UserGroupSaveDeleteListenerTest extends TestCase {
    private Group m_group;
    private Group m_phoneGroup;

    @Override
    protected void setUp() throws Exception {
        m_group = new Group();
        m_group.setResource(User.GROUP_RESOURCE_ID);
        m_group.setName("a");

        m_phoneGroup = new Group();
        m_phoneGroup.setName("b");
        m_phoneGroup.setResource(Phone.GROUP_RESOURCE_ID);
    }

    public void testOnSave() {
        final StringBuilder sb = new StringBuilder();
        UserGroupSaveDeleteListener ugsdl = new UserGroupSaveDeleteListener() {
            @Override
            protected void onUserGroupSave(Group group) {
                sb.append(group.getName());
            }
        };

        ugsdl.onSave(null);
        ugsdl.onSave(5);

        ugsdl.onSave(m_group);
        ugsdl.onSave(m_phoneGroup);

        assertEquals("a", sb.toString());
    }

    public void testOnDelete() {
        final StringBuilder sb = new StringBuilder();
        UserGroupSaveDeleteListener ugsdl = new UserGroupSaveDeleteListener() {
            @Override
            protected void onUserGroupDelete(Group group) {
                sb.append(group.getName());
            }
        };

        ugsdl.onDelete(null);
        ugsdl.onDelete(5);

        ugsdl.onDelete(m_group);
        ugsdl.onDelete(m_phoneGroup);

        assertEquals("a", sb.toString());
    }
}
