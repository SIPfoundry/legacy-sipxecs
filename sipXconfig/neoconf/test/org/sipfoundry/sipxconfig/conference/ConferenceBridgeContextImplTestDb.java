/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.Collections;
import java.util.List;

import org.dbunit.database.IDatabaseConnection;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;

public class ConferenceBridgeContextImplTestDb extends SipxDatabaseTestCase {

    private ConferenceBridgeContext m_context;
    private CoreContext m_coreContext;

    @Override
    protected void setUp() throws Exception {
        m_context = (ConferenceBridgeContext) TestHelper.getApplicationContext().getBean(
                ConferenceBridgeContext.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) TestHelper.getApplicationContext().getBean(CoreContext.CONTEXT_BEAN_NAME);

        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("conference/users.db.xml");
    }

    public void testGetBridges() throws Exception {
        TestHelper.insertFlat("conference/participants.db.xml");
        assertEquals(2, m_context.getBridges().size());
    }

    public void testGetBridgeByServer() throws Exception {
        TestHelper.insertFlat("conference/participants.db.xml");
        assertNull(m_context.getBridgeByServer("uknown"));
        assertEquals("bridge_name_2006", m_context.getBridgeByServer("host.example.com").getName());
    }

    public void testStore() throws Exception {
        IDatabaseConnection db = TestHelper.getConnection();

        Bridge bridge = new Bridge();
        bridge.setName("b1");
        Conference conference = new Conference();
        conference.setName("c1");
        bridge.addConference(conference);

        m_context.store(bridge);

        assertEquals(1, db.getRowCount("meetme_bridge"));
        assertEquals(1, db.getRowCount("meetme_conference"));
    }

    public void testRemoveBridges() throws Exception {
        IDatabaseConnection db = TestHelper.getConnection();
        TestHelper.insertFlat("conference/participants.db.xml");

        assertEquals(2, db.getRowCount("meetme_bridge"));
        assertEquals(3, db.getRowCount("meetme_conference"));

        m_context.removeBridges(Collections.singleton(new Integer(2005)));

        assertEquals(1, db.getRowCount("meetme_bridge"));
        assertEquals(1, db.getRowCount("meetme_conference"));
    }

    public void testRemoveConferences() throws Exception {
        IDatabaseConnection db = TestHelper.getConnection();
        TestHelper.insertFlat("conference/participants.db.xml");

        assertEquals(2, db.getRowCount("meetme_bridge"));
        assertEquals(3, db.getRowCount("meetme_conference"));

        m_context.removeConferences(Collections.singleton(new Integer(3002)));

        assertEquals(2, db.getRowCount("meetme_bridge"));
        assertEquals(2, db.getRowCount("meetme_conference"));
    }

    public void testLoadBridge() throws Exception {
        TestHelper.insertFlat("conference/participants.db.xml");
        Bridge bridge = m_context.loadBridge(new Integer(2006));

        assertEquals(1, bridge.getConferences().size());
    }

    public void testLoadConference() throws Exception {
        TestHelper.insertFlat("conference/participants.db.xml");
        Conference conference = m_context.loadConference(new Integer(3001));

        assertEquals("conf_name_3001", conference.getName());
        assertEquals("conf_desc_3001", conference.getDescription());
    }

    public void testFindConferenceByName() throws Exception {
        TestHelper.insertFlat("conference/participants.db.xml");
        Conference conference = m_context.findConferenceByName("conf_name_3001");

        assertEquals("conf_name_3001", conference.getName());
        assertEquals("conf_desc_3001", conference.getDescription());
    }

    public void testFindConferenceByOwner() throws Exception {
        TestHelper.insertFlat("conference/participants.db.xml");
        User owner = m_coreContext.loadUser(1002);
        List<Conference> ownerConferences = m_context.findConferencesByOwner(owner);

        assertEquals(3, ownerConferences.size());
        for (Conference conference : ownerConferences) {
            assertEquals(owner.getId(), conference.getOwner().getId());
        }
    }

    public void testClear() throws Exception {
        IDatabaseConnection db = TestHelper.getConnection();
        TestHelper.insertFlat("conference/participants.db.xml");

        assertTrue(0 < db.getRowCount("meetme_bridge"));
        assertTrue(0 < db.getRowCount("meetme_conference"));

        m_context.clear();

        assertEquals(0, db.getRowCount("meetme_bridge"));
        assertEquals(0, db.getRowCount("meetme_conference"));
    }

    public void testIsAliasInUse() throws Exception {
        TestHelper.getConnection();
        TestHelper.insertFlat("conference/participants.db.xml");

        // conference names are aliases
        assertTrue(m_context.isAliasInUse("conf_name_3001"));
        assertTrue(m_context.isAliasInUse("conf_name_3002"));
        assertTrue(m_context.isAliasInUse("conf_name_3003"));

        // conference extensions are aliases
        assertTrue(m_context.isAliasInUse("1699"));
        assertTrue(m_context.isAliasInUse("1700"));
        assertTrue(m_context.isAliasInUse("1701"));

        // we're not using this extension
        assertFalse(m_context.isAliasInUse("1702"));
    }

    public void testGetBeanIdsOfObjectsWithAlias() throws Exception {
        TestHelper.getConnection();
        TestHelper.insertFlat("conference/participants.db.xml");

        // conference names are aliases
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("conf_name_3001").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("conf_name_3002").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("conf_name_3003").size() == 1);

        // conference extensions are aliases
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("1699").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("1700").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("1701").size() == 1);

        // we're not using this extension
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("1702").size() == 0);
    }

    public void testValidate() throws Exception {
        TestHelper.getConnection();
        TestHelper.insertFlat("conference/participants.db.xml");

        // create a conference with a duplicate extension, should fail to validate
        Conference conf = new Conference();
        conf.setName("Appalachian");
        conf.setExtension("1699");
        try {
            m_context.validate(conf);
            fail("conference has duplicate extension but was validated anyway");
        } catch (UserException e) {
            // expected
        }

        // pick an unused extension, should be OK
        conf.setExtension("1800");
        m_context.validate(conf);
    }
}
