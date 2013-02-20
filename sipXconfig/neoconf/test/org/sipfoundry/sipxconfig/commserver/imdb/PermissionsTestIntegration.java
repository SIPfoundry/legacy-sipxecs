/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.commserver.imdb;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertCollectionCount;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectListFieldCount;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectPresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValuePresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdNotPresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdPresent;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.acccode.AuthCode;
import org.sipfoundry.sipxconfig.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;

import com.mongodb.QueryBuilder;

public class PermissionsTestIntegration extends ImdbTestCase {

    // needs to be adjusted every time a new permission is added
    private static int PERM_COUNT = 5;
    private static int SPEC_COUNT = SpecialUserType.values().length;
    User m_testUser;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_testUser = getCoreContext().newUser();
    }

    public void testGenerateEmpty() throws Exception {
        for (SpecialUserType u : SpecialUserType.values()) {
            SpecialUser su = new SpecialUser(u);
            getReplicationManager().replicateEntity(su, DataSet.PERMISSION);
        }

        // As PHONE_PROVISION does NOT require any permissions, don't count it.
        assertCollectionCount(getEntityCollection(), SPEC_COUNT - 1);
        // 5 permissions per special user

        for (SpecialUserType su : SpecialUserType.values()) {
            // As PHONE_PROVISION does NOT require any permissions, skip it.
            if (!su.equals(SpecialUserType.PHONE_PROVISION)) {
                assertObjectWithIdPresent(getEntityCollection(), su.getUserName());
                assertObjectListFieldCount(getEntityCollection(), su.getUserName(), MongoConstants.PERMISSIONS, PERM_COUNT);
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

        getReplicationManager().replicateEntity(callGroup1, DataSet.PERMISSION);
        getReplicationManager().replicateEntity(callGroup2, DataSet.PERMISSION);
        getReplicationManager().replicateEntity(callGroup3, DataSet.PERMISSION);

        assertObjectWithIdFieldValuePresent(getEntityCollection(), "CallGroup1", MongoConstants.IDENTITY, "sales@" + DOMAIN);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "CallGroup2", MongoConstants.IDENTITY, "marketing@" + DOMAIN);
        assertObjectWithIdNotPresent(getEntityCollection(), "CallGroup3");

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
        getReplicationManager().replicateEntity(m_testUser, DataSet.PERMISSION);

        assertObjectWithIdPresent(getEntityCollection(), "User1");
        assertObjectListFieldCount(getEntityCollection(), "User1", MongoConstants.PERMISSIONS, 8);
        QueryBuilder qb = QueryBuilder.start(MongoConstants.ID);
        qb.is("User1").and(MongoConstants.PERMISSIONS).size(4).and(MongoConstants.PERMISSIONS)
                .is(PermissionName.LOCAL_DIALING.getName()).is(PermissionName.VOICEMAIL.getName())
                .is(PermissionName.EXCHANGE_VOICEMAIL.getName()).is(PermissionName.MOBILE.getName());
        assertObjectPresent(getEntityCollection(), qb.get());
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
        getReplicationManager().replicateEntity(code, DataSet.PERMISSION);
        assertObjectWithIdPresent(getEntityCollection(), "AuthCode-1");
        QueryBuilder qb = QueryBuilder.start(MongoConstants.ID);
        qb.is("AuthCode-1").and(MongoConstants.PERMISSIONS).size(1).and(MongoConstants.PERMISSIONS)
                .is(PermissionName.NINEHUNDERED_DIALING.getName());
        assertObjectPresent(getEntityCollection(), qb.get());

        user.setPermission(PermissionName.NINEHUNDERED_DIALING, false);
        user.setPermission(PermissionName.INTERNATIONAL_DIALING, true);
        code.setInternalUser(user);

        getReplicationManager().replicateEntity(code, DataSet.PERMISSION);
        qb.is("AuthCode-1").and(MongoConstants.PERMISSIONS).size(1).and(MongoConstants.PERMISSIONS)
        .is(PermissionName.INTERNATIONAL_DIALING.getName());
        assertObjectPresent(getEntityCollection(), qb.get());
    }

}
