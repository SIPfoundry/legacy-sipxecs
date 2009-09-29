/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.permission;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;

public class PermissionTest extends TestCase {
    private Permission m_permission;

    public void setUp() {
        m_permission = new Permission();
        m_permission.setType(Permission.Type.CALL);
        m_permission.setId(15);
    }

    public void testGetSettingPath() {
        assertEquals("permission/call-handling/perm_15", m_permission.getSettingPath());
    }

    public void testGetSetting() {
        Setting setting = m_permission.getSetting();
        assertEquals(m_permission.getName(), setting.getName());
        assertTrue(setting.getType() instanceof BooleanSetting);

        Permission permission = new Permission();
        permission.setDefaultValue(true);
        assertEquals("ENABLE", permission.getSetting().getDefaultValue());
        assertTrue(permission.getSetting().getName().startsWith("perm"));
    }

    public void testGetPrimaryKey() throws Exception {
        assertEquals(15, m_permission.getPrimaryKey());
    }

    public void testLabelDescription() {
        assertEquals(m_permission.getLabel(), m_permission.getLabel(null));
        assertEquals(m_permission.getDescription(), m_permission.getDescription(null));
    }
}
