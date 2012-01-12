/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;
import static org.sipfoundry.sipxconfig.admin.commserver.imdb.MongoTestCaseHelper.assertObjectPresent;

import java.util.List;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.admin.authcode.AuthCode;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;

import com.mongodb.DBObject;

public class PermissionsTestIntegration extends ImdbTestCase {
    
    // needs to be adjusted every time a new permission is added
    private static int PERM_COUNT = 5;
    private Permissions m_permissionDataSet;
    private ReplicationManagerImpl m_replManager;
    User m_testUser;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_testUser = new User();
        m_testUser.setPermissionManager(getPermissionManager());
        m_testUser.setDomainManager(getDomainManager());
    }

    public void testGenerateEmpty() throws Exception {
        for (SpecialUserType u : SpecialUserType.values()) {
            SpecialUser su = new SpecialUser(u);
            DBObject suObj = m_replManager.findOrCreate(su);
            m_permissionDataSet.generate(su, suObj);
            if (u.equals(SpecialUserType.PHONE_PROVISION)) {
                assertNull(suObj.get(MongoConstants.PERMISSIONS));
            } else {
                assertEquals(PERM_COUNT, ((List) suObj.get(MongoConstants.PERMISSIONS)).size());
            }
        }
    }

    public void testCallGroupPerms() throws Exception {
        CallGroup callGroup1 = new CallGroup();
        callGroup1.setUniqueId(1);
        callGroup1.setName("sales");
        callGroup1.setEnabled(true);
        CallGroup callGroup2 = new CallGroup();
        callGroup2.setName("marketing");
        callGroup2.setEnabled(true);
        callGroup2.setUniqueId(2);
        CallGroup callGroup3 = new CallGroup();
        callGroup3.setName("disabled");
        callGroup3.setUniqueId(3);

        DBObject cg1Obj = m_replManager.findOrCreate(callGroup1);
        m_permissionDataSet.generate(callGroup1, cg1Obj);
        assertEquals("CallGroup1", cg1Obj.get(ID));
        assertEquals("sales@" + DOMAIN, cg1Obj.get(MongoConstants.IDENTITY));
        DBObject cg2Obj = m_replManager.findOrCreate(callGroup2);
        m_permissionDataSet.generate(callGroup2, cg2Obj);
        assertEquals("CallGroup2", cg2Obj.get(ID));
        assertEquals("marketing@" + DOMAIN, cg2Obj.get(MongoConstants.IDENTITY));
        DBObject cg3Obj = m_replManager.findOrCreate(callGroup3);
        assertEquals(false, m_permissionDataSet.generate(callGroup3, cg3Obj));
    }

    public void testAddUser() throws Exception {
        Group g = new Group();
        PermissionName.INTERNATIONAL_DIALING.setEnabled(g, false);
        PermissionName.LONG_DISTANCE_DIALING.setEnabled(g, false);
        PermissionName.TOLL_FREE_DIALING.setEnabled(g, false);
        PermissionName.LOCAL_DIALING.setEnabled(g, true);
        PermissionName.FREESWITH_VOICEMAIL.setEnabled(g, false);
        PermissionName.EXCHANGE_VOICEMAIL.setEnabled(g, true);

        m_testUser.addGroup(g);
        m_testUser.setUserName("goober");
        m_testUser.setUniqueId(1);
        DBObject userobj = m_replManager.findOrCreate(m_testUser);
        m_permissionDataSet.generate(m_testUser, userobj);
        assertEquals("User1", userobj.get(ID));
        List permissions = (List) userobj.get(MongoConstants.PERMISSIONS);
        assertEquals(8, permissions.size());
        assertTrue(permissions.contains("ExchangeUMVoicemailServer"));
        assertTrue(permissions.contains("LocalDialing"));
        assertTrue(permissions.contains("Mobile"));
        assertTrue(permissions.contains("Voicemail"));
        assertTrue(permissions.contains("music-on-hold"));
        assertTrue(permissions.contains("personal-auto-attendant"));
        assertTrue(permissions.contains("subscribe-to-presence"));
        assertTrue(permissions.contains("tui-change-pin"));
    }

    public void testAuthCodePermissions() {
        InternalUser user = new InternalUser();
        user.setSipPassword("123");
        user.setPintoken("11");
        user.setPermissionManager(getPermissionManager());
        user.setPermission(PermissionName.NINEHUNDERED_DIALING, true);
        user.setPermission(PermissionName.INTERNATIONAL_DIALING, false);
        user.setPermission(PermissionName.LOCAL_DIALING, false);
        user.setPermission(PermissionName.LONG_DISTANCE_DIALING, false);
        user.setPermission(PermissionName.MOBILE, false);
        user.setPermission(PermissionName.TOLL_FREE_DIALING, false);
        
        AuthCode code = new AuthCode();
        code.setInternalUser(user);
        DBObject codeObj = m_replManager.findOrCreate(code);
        m_permissionDataSet.generate(code, codeObj);
        assertEquals("AuthCode-1", codeObj.get(ID));
        List permissions = (List) codeObj.get(MongoConstants.PERMISSIONS);
        assertEquals(1, permissions.size());
        assertTrue(permissions.contains("900Dialing"));
        
        user.setPermission(PermissionName.NINEHUNDERED_DIALING, false);
        user.setPermission(PermissionName.INTERNATIONAL_DIALING, true);
        code.setInternalUser(user);
        m_permissionDataSet.generate(code, codeObj);
        permissions = (List) codeObj.get(MongoConstants.PERMISSIONS);
        assertEquals(1, permissions.size());
        assertTrue(permissions.contains("InternationalDialing"));
    }
    
    public void setPermissionDataSet(Permissions permissionDataSet) {
        m_permissionDataSet = permissionDataSet;
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replManager) {
        m_replManager = replManager;
    }

}
