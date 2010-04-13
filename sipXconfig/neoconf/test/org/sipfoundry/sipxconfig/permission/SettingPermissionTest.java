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

import org.sipfoundry.sipxconfig.setting.SettingImpl;

import junit.framework.TestCase;

public class SettingPermissionTest extends TestCase {

    public void testGetPrimaryKey() throws Exception {
        SettingImpl setting = new SettingImpl();
        setting.setName("voicemail");
        setting.setLabel("Voicemail");
        setting.setDescription("Voicemail Permissions");
        setting.setValue("DISABLED");

        SettingPermission permission = new SettingPermission(setting);

        assertEquals("voicemail", permission.getPrimaryKey());
        assertEquals("Voicemail", permission.getLabel());
        assertEquals("Voicemail Permissions", permission.getDescription());
        assertFalse(permission.getDefaultValue());
        assertEquals(Permission.Type.CALL, permission.getType());
        assertEquals(permission.getSettingPath(), "permission/call-handling/voicemail");
        assertEquals(permission.isBuiltIn(), false);

        SettingImpl anothersetting = new SettingImpl();
        anothersetting.setName("superadmin");
        anothersetting.setLabel("Superadmin permission");
        anothersetting.setDescription("This is a permission of a superadmin");
        anothersetting.setValue("DISABLED");

        SettingPermission anotherpermission = new SettingPermission(anothersetting,
                Permission.Type.APPLICATION, true);

        assertEquals("superadmin", anotherpermission.getPrimaryKey());
        assertEquals("Superadmin permission", anotherpermission.getLabel());
        assertEquals("This is a permission of a superadmin", anotherpermission.getDescription());
        assertFalse(anotherpermission.getDefaultValue());
        assertEquals(Permission.Type.APPLICATION, anotherpermission.getType());
        assertEquals(anotherpermission.getSettingPath(), "permission/application/superadmin");
        assertEquals(anotherpermission.isBuiltIn(), true);

    }
}
