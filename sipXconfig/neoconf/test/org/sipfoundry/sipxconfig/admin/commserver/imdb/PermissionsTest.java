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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.Arrays;
import java.util.Collections;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;

import com.mongodb.QueryBuilder;

public class PermissionsTest extends MongoTestCase {
    // needs to be adjusted every time a new permission is added
    private static int PERM_COUNT = 5;
    private static int SPEC_COUNT = SpecialUserType.values().length;
    private Permissions m_permissions;
    User m_testUser;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        m_testUser = new User();
        m_testUser.setPermissionManager(pm);
        DomainManager dm = getDomainManager();
        m_testUser.setDomainManager(dm);

        CoreContext coreContext;
        m_permissions = new Permissions();

        coreContext = getCoreContext();
        coreContext.loadInternalUsers();
        expectLastCall().andReturn(Collections.EMPTY_LIST).anyTimes();
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.EMPTY_LIST).anyTimes();
        coreContext.newUser();
        expectLastCall().andReturn(m_testUser).anyTimes();

        for (SpecialUserType u : SpecialUserType.values()) {
            coreContext.getSpecialUserAsSpecialUser(u);
            expectLastCall().andReturn(new SpecialUser(u)).anyTimes();
        }

        CallGroupContext callGroupContext = createMock(CallGroupContext.class);
        callGroupContext.getCallGroups();
        expectLastCall().andReturn(Collections.EMPTY_LIST);

        replay(coreContext, callGroupContext, dm);
        m_permissions.setCoreContext(coreContext);
        m_permissions.setCallGroupContext(callGroupContext);
        m_permissions.setDbCollection(getCollection());
    }

    public void testGenerateEmpty() throws Exception {
        m_permissions.generate();

        // As PHONE_PROVISION does NOT require any permissions, don't count it.
        assertCollectionCount(SPEC_COUNT - 1);
        // 5 permissions per special user

        for (SpecialUserType su : SpecialUserType.values()) {
            // As PHONE_PROVISION does NOT require any permissions, skip it.
            if (!su.equals(SpecialUserType.PHONE_PROVISION)) {
                assertObjectWithIdPresent(su.getUserName());
                assertObjectListFieldCount(su.getUserName(), Permissions.PERMISSIONS, PERM_COUNT);
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

        CallGroupContext callGroupContext = createMock(CallGroupContext.class);
        callGroupContext.getCallGroups();
        expectLastCall().andReturn(Arrays.asList(callGroup1, callGroup2, callGroup3));

        replay(callGroupContext);

        m_permissions.setCallGroupContext(callGroupContext);
        m_permissions.generate();

        assertObjectWithIdFieldValuePresent("CallGroup1", DataSetGenerator.IDENTITY, "sip:sales@" + DOMAIN);
        assertObjectWithIdFieldValuePresent("CallGroup2", DataSetGenerator.IDENTITY, "sip:marketing@" + DOMAIN);
        assertObjectWithIdNotPresent("CallGroup3");

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
        m_permissions.generate(m_testUser);

        assertObjectWithIdPresent("User1");
        assertObjectListFieldCount("User1", Permissions.PERMISSIONS, 4);
        QueryBuilder qb = QueryBuilder.start("id");
        qb.is("User1").and(Permissions.PERMISSIONS).size(4).and(Permissions.PERMISSIONS)
                .is(PermissionName.LOCAL_DIALING.getName()).is(PermissionName.VOICEMAIL.getName())
                .is(PermissionName.EXCHANGE_VOICEMAIL.getName()).is(PermissionName.MOBILE.getName());
        assertObjectPresent(qb.get());
    }
}
