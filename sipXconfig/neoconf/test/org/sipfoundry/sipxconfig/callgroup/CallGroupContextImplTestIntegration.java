/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.callgroup;


import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class CallGroupContextImplTestIntegration extends IntegrationTestCase {
    private CallGroupContext m_callGroupContext;
    private CoreContext m_coreContext;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        sql("common/TestUserSeed.sql");
        sql("domain/DomainSeed.sql");
        loadDataSetXml("callgroup/CallGroupSeed.xml");
    }

    public void testLoadCallGroup() throws Exception {
        CallGroup callGroup = m_callGroupContext.loadCallGroup(new Integer(1001));
        assertEquals("sales", callGroup.getName());
        assertTrue(callGroup.isEnabled());
        assertEquals("sales", callGroup.getName());
        assertEquals("401", callGroup.getExtension());
        assertEquals("sip:default@pingtel.com", callGroup.getFallbackDestination());
        assertFalse(callGroup.getVoicemailFallback());
        assertFalse(callGroup.getUserForward());
    }

    public void testGenerateSipPassword() throws Exception {
        CallGroup callGroup = m_callGroupContext.loadCallGroup(new Integer(1001));
        assertNull(callGroup.getSipPassword());
        m_callGroupContext.generateSipPasswords();
        callGroup = m_callGroupContext.loadCallGroup(new Integer(1001));
        assertTrue(callGroup.getSipPasswordHash("realm").length() > 0);
        callGroup = m_callGroupContext.loadCallGroup(new Integer(1001));
        assertTrue(callGroup.getSipPassword().length() > 0);
        assertTrue(callGroup.getSipPasswordHash("realm").length() > 0);
    }

    public void testGetCallGroups() throws Exception {
        List callGroups = m_callGroupContext.getCallGroups();
        assertEquals(2, callGroups.size());
        CallGroup callGroup = (CallGroup) callGroups.get(0);
        assertEquals("sales", callGroup.getName());
        assertTrue(callGroup.isEnabled());
        assertEquals("sales", callGroup.getName());
        assertEquals("401", callGroup.getExtension());
    }

    public void testLoadUserRing() throws Exception {
        CallGroup callGroup = m_callGroupContext.loadCallGroup(new Integer(1002));
        List userRings = callGroup.getRings();
        assertEquals(1, userRings.size());
        UserRing ring = (UserRing) userRings.get(0);
        assertSame(ring.getCallGroup(), callGroup);
        assertEquals(45, ring.getExpiration());
        assertEquals(AbstractRing.Type.DELAYED, ring.getType());
        assertEquals("testuser", ring.getUser().getUserName());

    }

    public void testStoreCallGroup() throws Exception {
        CallGroup group = new CallGroup();
        group.setName("kuku");
        group.setExtension("202");
        group.setEnabled(true);
        group.setVoicemailFallback(true);
        m_callGroupContext.saveCallGroup(group);
        commit();
        // table should have additional row now - 3
        assertEquals(3, countRowsInTable("call_group"));
    }

    public void testStoreCallGroupDuplicateName() throws Exception {
        CallGroup group = new CallGroup();
        group.setName("sales");
        group.setExtension("202");
        group.setEnabled(true);

        try {
            m_callGroupContext.saveCallGroup(group);
            fail("NameInUseException should be thrown");
        } catch (UserException e) {
            assertEquals(e.getMessage(),"&error.nameInUse.long");
        }
        // table should have the same number of rows as before
        assertEquals(2, countRowsInTable("call_group"));
    }

    public void testStoreCallGroupDuplicateExtension() throws Exception {
        CallGroup group = new CallGroup();
        group.setName("kuku");
        group.setExtension("401");
        group.setEnabled(true);
        try {
            m_callGroupContext.saveCallGroup(group);
            fail("ExtensionInUseException should be thrown");
        } catch (UserException e) {
            assertEquals(e.getMessage(),"&error.extensionInUse");
        }
        // table should have the same number of rows as before
        assertEquals(2, countRowsInTable("call_group"));
    }

    public void testRemoveCallGroups() throws Exception {
        List ids = Arrays.asList(new Integer[] {
            new Integer(1001), new Integer(1002)
        });
        m_callGroupContext.removeCallGroups(ids);
        // table should be empty now
        assertEquals(0, countRowsInTable("call_group"));
        assertEquals(0, countRowsInTable("user_ring"));
    }

    public void testRemoveCallGroupByAlias() throws Exception {
        m_callGroupContext.removeCallGroupByAlias("sales");
        m_callGroupContext.removeCallGroupByAlias("eng");
        // table should be empty now
        assertEquals(0, countRowsInTable("call_group"));
        assertEquals(0, countRowsInTable("user_ring"));
    }

    public void testDuplicateCallGroups() throws Exception {
        List ids = Arrays.asList(new Integer[] {
            new Integer(1001), new Integer(1002)
        });
        m_callGroupContext.duplicateCallGroups(ids);
        commit();
        // call groups table should have twice as many items
        assertEquals(4, countRowsInTable("call_group"));
        assertEquals(2, countRowsInTable("user_ring"));
    }

    public void testClear() throws Exception {
        m_callGroupContext.clear();
        commit();
        // make sure the tables are empty
        assertEquals(0, countRowsInTable("call_group"));
        assertEquals(0, countRowsInTable("user_ring"));
    }

    public void testEditUserRing() throws Exception {
        final Integer testExpiration = new Integer(12);

        CallGroup callGroup = m_callGroupContext.loadCallGroup(new Integer(1002));
        List userRings = callGroup.getRings();
        assertEquals(1, userRings.size());
        UserRing ring = (UserRing) userRings.get(0);
        ring.setExpiration(testExpiration.intValue());
        ring.setType(AbstractRing.Type.IMMEDIATE);
        m_callGroupContext.saveCallGroup(callGroup);
        commit();
        assertEquals(1, countRowsInTable("user_ring"));
        Map<String, Object> actual = db().queryForMap("select * from user_ring");
        assertEquals(testExpiration, actual.get("expiration"));
        assertEquals(AbstractRing.Type.IMMEDIATE.getName(), actual.get("ring_type"));
    }

    public void testRemoveUser() throws Exception {
        CallGroup callGroup = m_callGroupContext.loadCallGroup(new Integer(1002));
        List userRings = callGroup.getRings();
        assertEquals(1, userRings.size());

        UserRing ring = (UserRing) userRings.get(0);

        m_callGroupContext.removeUser(ring.getUser().getId());

        callGroup = m_callGroupContext.loadCallGroup(new Integer(1002));
        userRings = callGroup.getRings();
        assertTrue(userRings.isEmpty());
    }

    public void testDeleteUser() throws Exception {
        User user = m_coreContext.loadUser(new Integer(1000));
        assertEquals(1, countRowsInTable("user_ring"));
        m_coreContext.deleteUser(user);
        assertEquals(0, TestHelper.getConnection().createDataSet().getTable("user_ring").getRowCount());
    }

    public void testDeleteUserById() throws Exception {
        User user = m_coreContext.loadUser(new Integer(1000));
        assertEquals(1, countRowsInTable("user_ring"));
        m_coreContext.deleteUsers(Collections.singletonList(user.getId()));
        assertEquals(0, TestHelper.getConnection().createDataSet().getTable("user_ring").getRowCount());
    }

    public void testIsAliasInUse() {
        assertTrue(m_callGroupContext.isAliasInUse("sales"));
        assertTrue(m_callGroupContext.isAliasInUse("401"));
        assertTrue(m_callGroupContext.isAliasInUse("402"));
        assertTrue(m_callGroupContext.isAliasInUse("123456781"));
        assertTrue(m_callGroupContext.isAliasInUse("123456782"));
        assertFalse(m_callGroupContext.isAliasInUse("911"));
    }

    public void testGetBeanIdsOfObjectsWithAlias() {
        assertTrue(m_callGroupContext.getBeanIdsOfObjectsWithAlias("sales").size() == 1);
        assertTrue(m_callGroupContext.getBeanIdsOfObjectsWithAlias("401").size() == 1);
        assertTrue(m_callGroupContext.getBeanIdsOfObjectsWithAlias("402").size() == 1);
        assertTrue(m_callGroupContext.getBeanIdsOfObjectsWithAlias("123456781").size() == 1);
        assertTrue(m_callGroupContext.getBeanIdsOfObjectsWithAlias("123456782").size() == 1);
        assertTrue(m_callGroupContext.getBeanIdsOfObjectsWithAlias("911").size() == 0);
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

}
