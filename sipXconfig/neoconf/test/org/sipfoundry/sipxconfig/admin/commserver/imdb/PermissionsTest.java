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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class PermissionsTest extends TestCase {
    // needs to be adjusted every time a new permission is added
    private static int PERM_COUNT = 5;
    private static int SPEC_COUNT = SpecialUserType.values().length;

    public void testGenerateEmpty() throws Exception {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User testUser = new User();
        testUser.setPermissionManager(pm);

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.getDomainName();
        expectLastCall().andReturn("host.company.com");
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.EMPTY_LIST);
        coreContext.loadInternalUsers();
        expectLastCall().andReturn(Collections.EMPTY_LIST);
        coreContext.newUser();
        expectLastCall().andReturn(testUser).anyTimes();

        CallGroupContext callGroupContext = createMock(CallGroupContext.class);
        callGroupContext.getCallGroups();
        expectLastCall().andReturn(Collections.EMPTY_LIST);

        replay(coreContext, callGroupContext);

        Permissions permissions = new Permissions();
        permissions.setCoreContext(coreContext);
        permissions.setCallGroupContext(callGroupContext);

        List<Map<String, String>> items = permissions.generate();
        // As PHONE_PROVISION does NOT require any permissions, don't count it.
        assertEquals((SPEC_COUNT -1) * PERM_COUNT, items.size());
        // 5 permissions per special user

        for (SpecialUserType su : SpecialUserType.values()) {
            int i = su.ordinal();
            // As PHONE_PROVISION does NOT require any permissions, skip it.
            if (!su.equals(SpecialUserType.PHONE_PROVISION)) {
                String name = "sip:" + su.getUserName() + "@host.company.com";
                assertEquals(name, items.get(i * PERM_COUNT).get("identity"));
                assertEquals(name, items.get((i + 1) * PERM_COUNT - 1).get("identity"));
            }
        }
        verify(coreContext, callGroupContext);
    }

    public void testCallGroupPerms() throws Exception {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User testUser = new User();
        testUser.setPermissionManager(pm);

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.getDomainName();
        expectLastCall().andReturn("host.company.com");
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.EMPTY_LIST);
        coreContext.loadInternalUsers();
        expectLastCall().andReturn(Collections.EMPTY_LIST);
        coreContext.newUser();
        expectLastCall().andReturn(testUser).anyTimes();

        CallGroup callGroup1 = new CallGroup();
        callGroup1.setName("sales");
        callGroup1.setEnabled(true);
        CallGroup callGroup2 = new CallGroup();
        callGroup2.setName("marketing");
        callGroup2.setEnabled(true);
        CallGroup callGroup3 = new CallGroup();
        callGroup3.setName("disabled");

        CallGroupContext callGroupContext = createMock(CallGroupContext.class);
        callGroupContext.getCallGroups();
        expectLastCall().andReturn(Arrays.asList(callGroup1, callGroup2, callGroup3));

        replay(coreContext, callGroupContext);

        Permissions permissions = new Permissions();
        permissions.setCoreContext(coreContext);
        permissions.setCallGroupContext(callGroupContext);

        List<Map<String, String>> items = permissions.generate();

        // As PHONE_PROVISION does NOT require any permissions, don't count it.
        int start = (SPEC_COUNT - 1) * PERM_COUNT;

        assertEquals(start + 10, items.size());

        assertEquals("sip:sales@host.company.com", items.get(start + 0).get("identity"));
        assertEquals("sip:sales@host.company.com", items.get(start + 4).get("identity"));
        assertEquals("sip:marketing@host.company.com", items.get(start + 5).get("identity"));
        assertEquals("sip:marketing@host.company.com", items.get(start + 9).get("identity"));

        verify(coreContext, callGroupContext);
    }

    public void testAddUser() throws Exception {
        List<Map<String, String>> items = new ArrayList<Map<String, String>>();

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = new User();
        user.setPermissionManager(pm);

        Group g = new Group();
        PermissionName.INTERNATIONAL_DIALING.setEnabled(g, false);
        PermissionName.LONG_DISTANCE_DIALING.setEnabled(g, false);
        PermissionName.TOLL_FREE_DIALING.setEnabled(g, false);
        PermissionName.LOCAL_DIALING.setEnabled(g, true);
        PermissionName.FREESWITH_VOICEMAIL.setEnabled(g, false);
        PermissionName.EXCHANGE_VOICEMAIL.setEnabled(g, true);

        user.addGroup(g);
        user.setUserName("goober");

        Permissions permissions = new Permissions();
        permissions.addUser(items, user, "sipx.sipfoundry.org");

        assertEquals(PERM_COUNT, items.size());
        assertEquals("sip:goober@sipx.sipfoundry.org", items.get(0).get("identity"));
        assertEquals("LocalDialing", items.get(0).get("permission"));

        assertEquals("sip:goober@sipx.sipfoundry.org", items.get(3).get("identity"));
        assertEquals("ExchangeUMVoicemailServer", items.get(3).get("permission"));

        assertEquals("sip:~~vm~goober@sipx.sipfoundry.org", items.get(4).get("identity"));
        assertEquals("ExchangeUMVoicemailServer", items.get(4).get("permission"));
    }
}
