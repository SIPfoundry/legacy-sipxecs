/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.permission;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.ResetDialPlanTask;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.context.ApplicationContext;

import static org.sipfoundry.sipxconfig.permission.PermissionName.EXCHANGE_VOICEMAIL;
import static org.sipfoundry.sipxconfig.permission.PermissionName.FREESWITH_VOICEMAIL;
import static org.sipfoundry.sipxconfig.permission.PermissionName.SUPERADMIN;
import static org.sipfoundry.sipxconfig.permission.PermissionName.VOICEMAIL;

public class PermissionManagerImplTestDb extends SipxDatabaseTestCase {

    private PermissionManager m_manager;
    private DialPlanContext m_context;
    private ResetDialPlanTask m_resetDialPlanTask;

    @Override
    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_manager = (PermissionManager) app.getBean(PermissionManager.CONTEXT_BEAN_NAME);
        m_context = (DialPlanContext) app.getBean(DialPlanContext.CONTEXT_BEAN_NAME);
        m_resetDialPlanTask = (ResetDialPlanTask) app.getBean("resetDialPlanTask");
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testGetCallPermission() throws Exception {
        Permission permission = m_manager.getCallPermission(VOICEMAIL.getName());
        assertEquals(VOICEMAIL.getName(), permission.getName());
        assertEquals(VOICEMAIL.getName(), permission.getLabel());
        assertEquals(permission.isBuiltIn(), true);
        assertEquals("permission/call-handling/Voicemail", permission.getSettingPath());
        assertEquals("ENABLE", permission.getSetting().getDefaultValue());

        TestHelper.insertFlat("permission/permission.db.xml");

        permission = m_manager.getCallPermission(1002);
        assertNotNull(permission);

        assertEquals("kukuDescription", permission.getDescription());
        assertEquals("kukuLabel", permission.getLabel());

        Permission permissionNull = m_manager.getCallPermission("nonexistent");
        assertNull(permissionNull);
    }

    public void testAddCallPermission() throws Exception {
        Permission permission = new Permission();
        permission.setDescription("description");
        permission.setLabel("abc");

        m_manager.addCallPermission(permission);

        assertEquals(1, getConnection().getRowCount("permission", "where label = 'abc'"));
    }

    public void testAddCallPermissionDup() throws Exception {
        TestHelper.insertFlat("permission/permission.db.xml");

        Permission permission = new Permission();
        permission.setDescription("description");
        permission.setLabel("bongoLabel");

        try {
            m_manager.addCallPermission(permission);
            fail("");
        } catch (UserException e) {
            // ok - expected dup exception
        }

        Permission permission2 = m_manager.getCallPermission(1002);
        permission2.setDescription("new Description");
        m_manager.addCallPermission(permission2);
        assertEquals(2, getConnection().getRowCount("permission"));

        permission2.setLabel("new Label");
        m_manager.addCallPermission(permission2);
        assertEquals(2, getConnection().getRowCount("permission"));

        permission2 = m_manager.getCallPermission(1002);
        assertEquals(permission2.getDescription(), "new Description");
        assertEquals(permission2.getLabel(), "new Label");
    }

    public void testGetCallPermissions() throws Exception {
        Collection<Permission> permissions = m_manager.getPermissions(Permission.Type.CALL);
        int size = permissions.size();

        try {
            for (Permission p : permissions) {
                // There should only be builtIn call permissions at this point
                assertEquals(p.isBuiltIn(), true);
                if (p.getName().equals(VOICEMAIL.getName())) {
                    assertEquals(VOICEMAIL.getName(), p.getLabel());
                    assertEquals("ENABLE", p.getSetting().getDefaultValue());
                    assertEquals("permission/call-handling/Voicemail", p.getSettingPath());
                    throw new Exception();
                }
            }
            fail("No voicemail permission found");
        } catch (Exception exc) {
            // ok if superadmin permission is found
        }

        TestHelper.insertFlat("permission/permission.db.xml");
        permissions = m_manager.getPermissions(Permission.Type.CALL);
        assertEquals(size + 2, permissions.size());

        try {
            for (Permission p : permissions) {
                if (p.getName().equals("perm_1002")) {
                    assertEquals(p.isBuiltIn(), false);
                    assertEquals(true, p.getLabel().equals("kukuLabel"));
                    assertEquals(true, p.getDefaultValue());
                    assertEquals(true, p.getDescription().equals("kukuDescription"));
                    assertEquals(Permission.Type.CALL.getName(), p.getType().getName());
                    throw new Exception();
                }
            }
            fail("No 1002 permission found");
        } catch (Exception exc) {
            // ok if superadmin permission is found
        }
    }

    public void testPermisionModel() throws Exception {
        Setting setting = m_manager.getPermissionModel();
        Collection<Setting> settingsBefore = setting.getSetting(Permission.CALL_PERMISSION_PATH).getValues();

        TestHelper.insertFlat("permission/permission.db.xml");
        setting = m_manager.getPermissionModel();
        Collection<Setting> settingsAfter = setting.getSetting(Permission.CALL_PERMISSION_PATH).getValues();
        assertEquals(settingsBefore.size() + 2, settingsAfter.size());
    }

    public void testRemoveCallPermissions() throws Exception {
        Integer[] names = {
            1002, 1001
        };
        TestHelper.insertFlat("permission/permission.db.xml");

        m_manager.removeCallPermissions(Arrays.asList(names));
        assertEquals(0, getConnection().getRowCount("permission"));
    }

    public void testRulesWithCustomPermission() throws Exception {
        m_resetDialPlanTask.reset(true);

        Permission permission = new Permission();
        permission.setType(Permission.Type.CALL);
        permission.setLabel("bongo3");

        m_manager.addCallPermission(permission);

        CustomDialingRule rule = new CustomDialingRule();
        rule.setName("a2");
        rule.setPermissionNames(Collections.singletonList(permission.getName()));

        m_context.storeRule(rule);

        CustomDialingRule loaded = (CustomDialingRule) m_context.load(DialingRule.class, rule.getId());
        assertEquals(1, rule.getPermissionNames().size());
        List<Permission> loadedPermissions = loaded.getPermissions();
        assertEquals(1, loadedPermissions.size());
        assertEquals(permission, loadedPermissions.get(0));
    }

    public void testGetApplicationPermissions() throws Exception {

        Collection<Permission> permissions = m_manager.getPermissions(Permission.Type.APPLICATION);
        int size = permissions.size();
        assertEquals(permissions.isEmpty(), false);

        try {
            for (Permission p : permissions) {
                // Application permissions are all BuiltIn
                assertEquals(p.isBuiltIn(), true);
                if (p.getName().equals(SUPERADMIN.getName())) {
                    assertEquals(SUPERADMIN.getName(), p.getLabel());
                    assertEquals("DISABLE", p.getSetting().getDefaultValue());
                    assertEquals("permission/application/superadmin", p.getSettingPath());
                    throw new Exception();
                }
            }
            fail("No superadmin permission found");
        } catch (Exception exc) {
            // ok if superadmin permission is found
        }
        TestHelper.insertFlat("permission/permission.db.xml");

        permissions = m_manager.getPermissions(Permission.Type.APPLICATION);
        assertEquals(size, permissions.size());

    }

    public void testGetVMServerPermissions() throws Exception {

        Collection<Permission> permissions = m_manager.getPermissions(Permission.Type.VOICEMAIL_SERVER);
        int size = permissions.size();
        assertEquals(permissions.isEmpty(), false);

        try {
            for (Permission p : permissions) {
                // Application permissions are all BuiltIn
                assertEquals(p.isBuiltIn(), true);
                if (p.getName().equals(EXCHANGE_VOICEMAIL.getName())) {
                    assertEquals(EXCHANGE_VOICEMAIL.getName(), p.getLabel());
                    assertEquals("DISABLE", p.getSetting().getDefaultValue());
                    assertEquals("permission/voicemail-server/ExchangeUMVoicemailServer", p.getSettingPath());
                    throw new Exception();
                }
            }
            fail("No ExchangeUMVoicemailServer permission found");
        } catch (Exception exc) {
            // ok if ExchangeUMVoicemailServer permission is found
        }
        TestHelper.insertFlat("permission/permission.db.xml");

        permissions = m_manager.getPermissions(Permission.Type.VOICEMAIL_SERVER);
        assertEquals(size, permissions.size());

    }

    public void testGetAllPermissions() throws Exception {

        Collection<Permission> permissions = m_manager.getPermissions();
        int size = permissions.size();
        assertEquals(permissions.isEmpty(), false);

        try {
            for (Permission p : permissions) {
                // Application permissions are all BuiltIn
                assertEquals(p.isBuiltIn(), true);
                if (p.getName().equals(SUPERADMIN.getName())) {
                    assertEquals(SUPERADMIN.getName(), p.getLabel());
                    assertEquals("DISABLE", p.getSetting().getDefaultValue());
                    assertEquals("permission/application/superadmin", p.getSettingPath());
                    throw new Exception();
                }
            }
            fail("No superadmin permission found");
        } catch (Exception exc) {
            // ok if superadmin permission is found
        }

        TestHelper.insertFlat("permission/permission.db.xml");

        permissions = m_manager.getPermissions();
        assertEquals(size + 2, permissions.size());

        try {
            for (Permission p : permissions) {
                // There should only be builtIn call permissions at this point
                assertEquals(p.isBuiltIn(), true);
                if (p.getName().equals(VOICEMAIL.getName())) {
                    assertEquals(VOICEMAIL.getName(), p.getLabel());
                    assertEquals("ENABLE", p.getSetting().getDefaultValue());
                    assertEquals("permission/call-handling/Voicemail", p.getSettingPath());
                    throw new Exception();
                }
            }
            fail("No voicemail permission found");
        } catch (Exception exc) {
            // ok if superadmin permission is found
        }

        try {
            for (Permission p : permissions) {
                if (p.getName().equals("perm_1001")) {
                    assertEquals(p.isBuiltIn(), false);
                    assertEquals(true, p.getLabel().equals("bongoLabel"));
                    assertEquals(false, p.getDefaultValue());
                    assertEquals(true, p.getDescription().equals("bongoDescription"));
                    assertEquals(Permission.Type.CALL.getName(), p.getType().getName());
                    throw new Exception();
                }
            }
            fail("No 1001 permission found");
        } catch (Exception exc) {
            // ok if superadmin permission is found
        }
    }

    public void testGetApplicationPermissionByName() throws Exception {

        Permission p = m_manager.getPermissionByName(Permission.Type.APPLICATION, SUPERADMIN.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(SUPERADMIN.getName(), p.getLabel());
        assertEquals("DISABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/application/superadmin", p.getSettingPath());

        TestHelper.insertFlat("permission/permission.db.xml");

        p = m_manager.getPermissionByName(Permission.Type.APPLICATION, SUPERADMIN.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(SUPERADMIN.getName(), p.getLabel());
        assertEquals("DISABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/application/superadmin", p.getSettingPath());
    }

    public void testGetCallPermissionByName() throws Exception {

        Permission p = m_manager.getPermissionByName(Permission.Type.CALL, VOICEMAIL.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(VOICEMAIL.getName(), p.getLabel());
        assertEquals("ENABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/call-handling/Voicemail", p.getSettingPath());

        TestHelper.insertFlat("permission/permission.db.xml");

        p = m_manager.getPermissionByName(Permission.Type.CALL, "perm_1001");
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), false);
        assertEquals(true, p.getLabel().equals("bongoLabel"));
        assertEquals(false, p.getDefaultValue());
        assertEquals(true, p.getDescription().equals("bongoDescription"));
        assertEquals(Permission.Type.CALL.getName(), p.getType().getName());
    }

    public void testGetVMServerPermissionByName() throws Exception {

        Permission p = m_manager
                .getPermissionByName(Permission.Type.VOICEMAIL_SERVER, FREESWITH_VOICEMAIL.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(FREESWITH_VOICEMAIL.getName(), p.getLabel());
        assertEquals("ENABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/voicemail-server/FreeswitchVoicemailServer", p.getSettingPath());

        TestHelper.insertFlat("permission/permission.db.xml");

        p = m_manager.getPermissionByName(Permission.Type.VOICEMAIL_SERVER, FREESWITH_VOICEMAIL.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(FREESWITH_VOICEMAIL.getName(), p.getLabel());
        assertEquals("ENABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/voicemail-server/FreeswitchVoicemailServer", p.getSettingPath());
    }

    public void testGetAnyPermissionByName() throws Exception {

        Permission p = m_manager.getPermissionByName(SUPERADMIN.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(true, p.getLabel().equals(SUPERADMIN.getName()));
        assertEquals("DISABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/application/superadmin", p.getSettingPath());

        p = m_manager.getPermissionByName(VOICEMAIL.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(true, p.getLabel().equals(VOICEMAIL.getName()));
        assertEquals("ENABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/call-handling/Voicemail", p.getSettingPath());

        TestHelper.insertFlat("permission/permission.db.xml");

        p = m_manager.getPermissionByName("perm_1001");
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), false);
        assertEquals(true, p.getLabel().equals("bongoLabel"));
        assertEquals(false, p.getDefaultValue());
        assertEquals(true, p.getDescription().equals("bongoDescription"));
        assertEquals(Permission.Type.CALL.getName(), p.getType().getName());
    }

    public void testClear() throws Exception {

        assertEquals(0, getConnection().getRowCount("permission"));

        TestHelper.insertFlat("permission/permission.db.xml");

        Permission permission = new Permission();
        permission.setDescription("description");
        permission.setLabel("abc");
        m_manager.addCallPermission(permission);

        assertEquals(3, getConnection().getRowCount("permission"));

        m_manager.clear();

        assertEquals(0, getConnection().getRowCount("permission"));
    }
}
