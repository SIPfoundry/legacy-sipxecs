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


import static org.sipfoundry.sipxconfig.permission.PermissionName.EXCHANGE_VOICEMAIL;
import static org.sipfoundry.sipxconfig.permission.PermissionName.FREESWITH_VOICEMAIL;
import static org.sipfoundry.sipxconfig.permission.PermissionName.SUPERADMIN;
import static org.sipfoundry.sipxconfig.permission.PermissionName.VOICEMAIL;

import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.dialplan.DialPlanSetup;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class PermissionManagerImplTestIntegration extends IntegrationTestCase {
    private PermissionManager m_permissionManager;
    private DialPlanContext m_dialPlanContext;
    private DialPlanSetup m_dialPlanSetup;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        m_permissionManager.setCustomPermissions(null);
        m_permissionManager.setPermissions(null);
    }

    public void testGetCallPermission() throws Exception {
        Permission permission = m_permissionManager.getCallPermission(VOICEMAIL.getName());
        assertEquals(VOICEMAIL.getName(), permission.getName());
        assertEquals(VOICEMAIL.getName(), permission.getLabel());
        assertEquals(permission.isBuiltIn(), true);
        assertEquals("permission/call-handling/Voicemail", permission.getSettingPath());
        assertEquals("ENABLE", permission.getSetting().getDefaultValue());

        sql("permission/permission.sql");

        permission = m_permissionManager.getCallPermission(1002);
        assertNotNull(permission);

        assertEquals("kukuDescription", permission.getDescription());
        assertEquals("kukuLabel", permission.getLabel());

        Permission permissionNull = m_permissionManager.getCallPermission("nonexistent");
        assertNull(permissionNull);
    }

    public void testAddCallPermission() throws Exception {
        Permission permission = new Permission();
        permission.setDescription("description");
        permission.setLabel("abc");
        sql("commserver/locations.sql");
        m_permissionManager.saveCallPermission(permission);
        commit();
        assertEquals(1, db().queryForLong("select count(*) from permission where label = 'abc'"));
    }

    public void testAddCallPermissionDup() throws Exception {
        sql("permission/permission.sql");

        Permission permission = new Permission();
        permission.setDescription("description");
        permission.setLabel("bongoLabel");

        try {
            m_permissionManager.saveCallPermission(permission);
            fail("");
        } catch (UserException e) {
            // ok - expected dup exception
        }

        Permission permission2 = m_permissionManager.getCallPermission(1002);
        permission2.setDescription("new Description");
        m_permissionManager.saveCallPermission(permission2);
        assertEquals(2, getConnection().getRowCount("permission"));

        permission2.setLabel("new Label");
        m_permissionManager.saveCallPermission(permission2);
        assertEquals(2, getConnection().getRowCount("permission"));

        permission2 = m_permissionManager.getCallPermission(1002);
        assertEquals(permission2.getDescription(), "new Description");
        assertEquals(permission2.getLabel(), "new Label");
    }

    public void testGetCallPermissions() throws Exception {
        Collection<Permission> permissions = m_permissionManager.getPermissions(Permission.Type.CALL);
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

        sql("permission/permission.sql");
        // set null here to simulate save
        m_permissionManager.setCustomPermissions(null);
        permissions = m_permissionManager.getPermissions(Permission.Type.CALL);
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
        Setting setting = m_permissionManager.getPermissionModel();
        Collection<Setting> settingsBefore = setting.getSetting(Permission.CALL_PERMISSION_PATH).getValues();

        sql("permission/permission.sql");
        // set null here to simulate save
        m_permissionManager.setCustomPermissions(null);
        setting = m_permissionManager.getPermissionModel();
        Collection<Setting> settingsAfter = setting.getSetting(Permission.CALL_PERMISSION_PATH).getValues();
        assertEquals(settingsBefore.size() + 2, settingsAfter.size());
    }

    public void testRemoveCallPermissions() throws Exception {
        sql("permission/permission.sql");
        Integer[] names = {
            1002, 1001
        };

        m_permissionManager.deleteCallPermission(m_permissionManager.load(Permission.class, 1001));
        m_permissionManager.deleteCallPermission(m_permissionManager.load(Permission.class, 1002));
        commit();
        assertEquals(0, countRowsInTable("permission"));
    }

    public void testRulesWithCustomPermission() throws Exception {
        m_dialPlanSetup.setupDefaultRegion();
        sql("commserver/locations.sql");
        Permission permission = new Permission();
        permission.setType(Permission.Type.CALL);
        permission.setLabel("bongo3");

        m_permissionManager.saveCallPermission(permission);

        CustomDialingRule rule = new CustomDialingRule();
        rule.setName("a2");
        rule.setPermissionNames(Collections.singletonList(permission.getName()));

        m_dialPlanContext.storeRule(rule);
        commit();
        CustomDialingRule loaded = (CustomDialingRule) m_dialPlanContext.load(DialingRule.class, rule.getId());
        assertEquals(1, rule.getPermissionNames().size());
        List<Permission> loadedPermissions = loaded.getPermissions();
        assertEquals(1, loadedPermissions.size());
        assertEquals(permission, loadedPermissions.get(0));
    }

    public void testGetApplicationPermissions() throws Exception {

        Collection<Permission> permissions = m_permissionManager.getPermissions(Permission.Type.APPLICATION);
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
        sql("permission/permission.sql");

        permissions = m_permissionManager.getPermissions(Permission.Type.APPLICATION);
        assertEquals(size, permissions.size());

    }

    public void testGetVMServerPermissions() throws Exception {

        Collection<Permission> permissions = m_permissionManager.getPermissions(Permission.Type.VOICEMAIL_SERVER);
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
        sql("permission/permission.sql");

        permissions = m_permissionManager.getPermissions(Permission.Type.VOICEMAIL_SERVER);
        assertEquals(size, permissions.size());

    }

    public void testGetAllPermissions() throws Exception {
        Collection<Permission> permissions = m_permissionManager.getPermissions();
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

        sql("permission/permission.sql");
        // set null here to simulate save
        m_permissionManager.setCustomPermissions(null);
        m_permissionManager.setPermissions(null);
        permissions = m_permissionManager.getPermissions();
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
        Permission p = m_permissionManager.getPermissionByName(Permission.Type.APPLICATION, SUPERADMIN.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(SUPERADMIN.getName(), p.getLabel());
        assertEquals("DISABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/application/superadmin", p.getSettingPath());
    }
    
    
    public void testGetApplicationPermissionByNameFromDb() throws Exception {
        sql("permission/permission.sql");

        Permission p = m_permissionManager.getPermissionByName(Permission.Type.APPLICATION, SUPERADMIN.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(SUPERADMIN.getName(), p.getLabel());
        assertEquals("DISABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/application/superadmin", p.getSettingPath());
    }

    public void testGetCallPermissionByName() throws Exception {
        Permission p = m_permissionManager.getPermissionByName(Permission.Type.CALL, VOICEMAIL.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(VOICEMAIL.getName(), p.getLabel());
        assertEquals("ENABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/call-handling/Voicemail", p.getSettingPath());
    }

    public void testGetCallPermissionByNameFromDb() throws Exception {
        sql("permission/permission.sql");

        Permission p = m_permissionManager.getPermissionByName(Permission.Type.CALL, "perm_1001");
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), false);
        assertEquals(true, p.getLabel().equals("bongoLabel"));
        assertEquals(false, p.getDefaultValue());
        assertEquals(true, p.getDescription().equals("bongoDescription"));
        assertEquals(Permission.Type.CALL.getName(), p.getType().getName());
    }

    public void testGetVMServerPermissionByName() throws Exception {
        Permission p = m_permissionManager
                .getPermissionByName(Permission.Type.VOICEMAIL_SERVER, FREESWITH_VOICEMAIL.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(FREESWITH_VOICEMAIL.getName(), p.getLabel());
        assertEquals("ENABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/voicemail-server/FreeswitchVoicemailServer", p.getSettingPath());
    }
    
    public void testGetVMServerPermissionByNameFromDb() throws Exception {
        sql("permission/permission.sql");

        Permission p = m_permissionManager.getPermissionByName(Permission.Type.VOICEMAIL_SERVER, FREESWITH_VOICEMAIL.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(FREESWITH_VOICEMAIL.getName(), p.getLabel());
        assertEquals("ENABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/voicemail-server/FreeswitchVoicemailServer", p.getSettingPath());
    }

    public void testGetAnyPermissionByName() throws Exception {
        Permission p = m_permissionManager.getPermissionByName(SUPERADMIN.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(true, p.getLabel().equals(SUPERADMIN.getName()));
        assertEquals("DISABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/application/superadmin", p.getSettingPath());

        p = m_permissionManager.getPermissionByName(VOICEMAIL.getName());
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), true);
        assertEquals(true, p.getLabel().equals(VOICEMAIL.getName()));
        assertEquals("ENABLE", p.getSetting().getDefaultValue());
        assertEquals("permission/call-handling/Voicemail", p.getSettingPath());
    }
    
    public void testPermissionFromDb() throws Exception {
        sql("permission/permission.sql");
        Permission p = m_permissionManager.getPermissionByName("perm_1001");
        assertNotNull(p);
        assertEquals(p.isBuiltIn(), false);
        assertEquals(true, p.getLabel().equals("bongoLabel"));
        assertEquals(false, p.getDefaultValue());
        assertEquals(true, p.getDescription().equals("bongoDescription"));
        assertEquals(Permission.Type.CALL.getName(), p.getType().getName());
    }

    public void testSave() throws Exception {
        sql("permission/permission.sql");

        Permission permission = new Permission();
        permission.setDescription("description");
        permission.setLabel("abc");
        m_permissionManager.saveCallPermission(permission);
        commit();

        assertEquals(3, countRowsInTable("permission"));
    }
    
    public void testClear() throws Exception {
        sql("permission/permission.sql");
        m_permissionManager.clear();
        commit();
        assertEquals(0, countRowsInTable("permission"));
    }

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public void setDialPlanSetup(DialPlanSetup dialPlanSetup) {
        m_dialPlanSetup = dialPlanSetup;
    }
}
