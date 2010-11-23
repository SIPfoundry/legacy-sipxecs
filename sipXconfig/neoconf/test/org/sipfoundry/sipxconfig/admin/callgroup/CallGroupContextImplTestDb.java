/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.context.ApplicationContext;

public class CallGroupContextImplTestDb extends SipxDatabaseTestCase {

    private CallGroupContext m_context;

    private CoreContext m_coreContext;

    @Override
    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (CallGroupContext) appContext.getBean(CallGroupContext.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) appContext.getBean(CoreContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsert("admin/callgroup/CallGroupSeed.xml");
    }

    public void testLoadCallGroup() throws Exception {
        CallGroup callGroup = m_context.loadCallGroup(new Integer(1001));
        assertEquals("sales", callGroup.getName());
        assertTrue(callGroup.isEnabled());
        assertEquals("sales", callGroup.getName());
        assertEquals("401", callGroup.getExtension());
        assertEquals("sip:default@pingtel.com", callGroup.getFallbackDestination());
        assertFalse(callGroup.getVoicemailFallback());
        assertFalse(callGroup.getUserForward());
    }

    public void testGenerateSipPassword() throws Exception {
        CallGroup callGroup = m_context.loadCallGroup(new Integer(1001));
        assertNull(callGroup.getSipPassword());
        m_context.generateSipPasswords();
        callGroup = m_context.loadCallGroup(new Integer(1001));
        assertTrue(callGroup.getSipPasswordHash("realm").length() > 0);
        callGroup = m_context.loadCallGroup(new Integer(1001));
        assertTrue(callGroup.getSipPassword().length() > 0);
        assertTrue(callGroup.getSipPasswordHash("realm").length() > 0);
    }

    public void testGetCallGroups() throws Exception {
        List callGroups = m_context.getCallGroups();
        assertEquals(2, callGroups.size());
        CallGroup callGroup = (CallGroup) callGroups.get(0);
        assertEquals("sales", callGroup.getName());
        assertTrue(callGroup.isEnabled());
        assertEquals("sales", callGroup.getName());
        assertEquals("401", callGroup.getExtension());
    }

    public void testLoadUserRing() throws Exception {
        CallGroup callGroup = m_context.loadCallGroup(new Integer(1002));
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
        m_context.storeCallGroup(group);
        // table should have additional row now - 3
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("call_group");
        assertEquals(3, tableCallGroup.getRowCount());
    }

    public void testStoreCallGroupDuplicateName() throws Exception {
        CallGroup group = new CallGroup();
        group.setName("sales");
        group.setExtension("202");
        group.setEnabled(true);

        try {
            m_context.storeCallGroup(group);
            fail("NameInUseException should be thrown");
        } catch (UserException e) {
            assertTrue(e.getMessage().indexOf("sales") > 0);
        }
        // table should have the same number of rows as before
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("call_group");
        assertEquals(2, tableCallGroup.getRowCount());
    }

    public void testStoreCallGroupDuplicateExtension() throws Exception {
        CallGroup group = new CallGroup();
        group.setName("kuku");
        group.setExtension("401");
        group.setEnabled(true);
        try {
            m_context.storeCallGroup(group);
            fail("ExtensionInUseException should be thrown");
        } catch (UserException e) {
            assertTrue(e.getMessage().indexOf("401") > 0);
        }
        // table should have the same number of rows as before
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("call_group");
        assertEquals(2, tableCallGroup.getRowCount());
    }

    public void testRemoveCallGroups() throws Exception {
        List ids = Arrays.asList(new Integer[] {
            new Integer(1001), new Integer(1002)
        });
        m_context.removeCallGroups(ids);
        // table should be empty now
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("call_group");
        assertEquals(0, tableCallGroup.getRowCount());
        ITable tableUserRing = TestHelper.getConnection().createDataSet().getTable("user_ring");
        assertEquals(0, tableUserRing.getRowCount());
    }

    public void testDuplicateCallGroups() throws Exception {
        List ids = Arrays.asList(new Integer[] {
            new Integer(1001), new Integer(1002)
        });
        m_context.duplicateCallGroups(ids);
        // call groups table should have twice as many items
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("call_group");
        assertEquals(4, tableCallGroup.getRowCount());
        // and rings table should have twice as many items
        ITable tableUserRing = TestHelper.getConnection().createDataSet().getTable("user_ring");
        assertEquals(2, tableUserRing.getRowCount());
    }

    public void testClear() throws Exception {
        m_context.clear();
        // make sure the tables are empty
        ITable tableCallGroup = TestHelper.getConnection().createDataSet().getTable("call_group");
        assertEquals(0, tableCallGroup.getRowCount());
        ITable tableUserRing = TestHelper.getConnection().createDataSet().getTable("user_ring");
        assertEquals(0, tableUserRing.getRowCount());
    }

    public void testEditUserRing() throws Exception {
        final Integer testExpiration = new Integer(12);

        CallGroup callGroup = m_context.loadCallGroup(new Integer(1002));
        List userRings = callGroup.getRings();
        assertEquals(1, userRings.size());
        UserRing ring = (UserRing) userRings.get(0);
        ring.setExpiration(testExpiration.intValue());
        ring.setType(AbstractRing.Type.IMMEDIATE);
        m_context.storeCallGroup(callGroup);

        ITable tableUserRing = TestHelper.getConnection().createDataSet().getTable("user_ring");
        assertEquals(1, tableUserRing.getRowCount());
        assertEquals(testExpiration, tableUserRing.getValue(0, "expiration"));
        assertEquals(AbstractRing.Type.IMMEDIATE.getName(), tableUserRing.getValue(0, "ring_type"));
    }

    public void testGenerateAliases() throws Exception {
        Collection<AliasMapping> aliases = m_context.getAliasMappings();
        assertNotNull(aliases);
        for (AliasMapping aliasMapping : aliases) {
            System.out.println(aliasMapping.getIdentity());

        }
        assertEquals(3, aliases.size());

        Iterator<AliasMapping> i = aliases.iterator();
        AliasMapping aliasMapping = i.next();
        assertTrue(aliasMapping.getIdentity().startsWith("sales"));
        assertTrue(aliasMapping.getContact().startsWith("<sip:default@pingtel.com>;q="));

        aliasMapping = i.next();
        assertTrue(aliasMapping.getIdentity().startsWith("401"));
        assertTrue(aliasMapping.getContact().startsWith("sales"));

        aliasMapping = i.next();
        assertTrue(aliasMapping.getIdentity().startsWith("123456781"));
        assertTrue(aliasMapping.getContact().startsWith("sales"));
    }

    public void testRemoveUser() throws Exception {
        CallGroup callGroup = m_context.loadCallGroup(new Integer(1002));
        List userRings = callGroup.getRings();
        assertEquals(1, userRings.size());

        UserRing ring = (UserRing) userRings.get(0);

        m_context.removeUser(ring.getUser().getId());

        callGroup = m_context.loadCallGroup(new Integer(1002));
        userRings = callGroup.getRings();
        assertTrue(userRings.isEmpty());
    }

    public void testDeleteUser() throws Exception {
        User user = m_coreContext.loadUser(new Integer(1000));
        ITable rings = TestHelper.getConnection().createDataSet().getTable("user_ring");

        assertEquals(1, rings.getRowCount());
        m_coreContext.deleteUser(user);

        rings = TestHelper.getConnection().createDataSet().getTable("user_ring");
        assertEquals(0, rings.getRowCount());
    }

    public void testDeleteUserById() throws Exception {
        User user = m_coreContext.loadUser(new Integer(1000));
        ITable rings = TestHelper.getConnection().createDataSet().getTable("user_ring");

        assertEquals(1, rings.getRowCount());
        m_coreContext.deleteUsers(Collections.singletonList(user.getId()));

        rings = TestHelper.getConnection().createDataSet().getTable("user_ring");
        assertEquals(0, rings.getRowCount());
    }

    public void testIsAliasInUse() {
        assertTrue(m_context.isAliasInUse("sales"));
        assertTrue(m_context.isAliasInUse("401"));
        assertTrue(m_context.isAliasInUse("402"));
        assertTrue(m_context.isAliasInUse("123456781"));
        assertTrue(m_context.isAliasInUse("123456782"));
        assertFalse(m_context.isAliasInUse("911"));
    }

    public void testGetBeanIdsOfObjectsWithAlias() {
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("sales").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("401").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("402").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("123456781").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("123456782").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("911").size() == 0);
    }

}
