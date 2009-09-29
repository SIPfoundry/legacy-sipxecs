/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.util.Arrays;
import java.util.Set;
import java.util.TreeSet;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;

public class UserBuilderTest extends TestCase {
    private UserBuilder m_builder;
    private org.sipfoundry.sipxconfig.common.User m_myUser;
    private User m_apiUser;

    protected void setUp() {
        m_builder = new UserBuilder();
        m_myUser = new org.sipfoundry.sipxconfig.common.User();
        m_apiUser = new User();
    }

    public void testFromApi() {
        m_apiUser.setAliases(new String[] {
            "one", "two"
        });
        ApiBeanUtil.toMyObject(m_builder, m_myUser, m_apiUser);
        assertEquals("one two", m_myUser.getAliasesString());
    }

    public void testToApi() {
        Set<Permission> permissionNames = new TreeSet<Permission>();
        IMocksControl permissionManagerControl = EasyMock.createStrictControl();
        PermissionManager permissionManager = permissionManagerControl
                .createMock(PermissionManager.class);
        permissionManager.getPermissions();
        permissionManagerControl.andReturn(permissionNames);
        permissionManagerControl.replay();

        m_myUser.setPermissionManager(permissionManager);
        m_myUser.setAliasesString("one two");
        ApiBeanUtil.toApiObject(m_builder, m_apiUser, m_myUser);

        String[] aliases = m_apiUser.getAliases();
        assertEquals(2, aliases.length);
        // NOTE: order is not important
        Arrays.sort(aliases);
        assertEquals("one", aliases[0]);
        assertEquals("two", aliases[1]);

        String[] permissions = m_apiUser.getPermissions();
        assertNull(permissions);

        permissionManagerControl.verify();
    }
}
