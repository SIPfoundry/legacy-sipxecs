/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.SameExtensionException;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ConferenceBridgeContextImplTestIntegration extends IntegrationTestCase {
    private ConferenceBridgeContext m_context;
    private ConferenceBridgeContextImpl m_contextImpl;
    private CoreContext m_core;
    private LocationsManager m_locations;
    private DomainManager m_domainManager;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void setCoreContext(CoreContext core) {
        m_core = core;
    }

    public void setLocationsManager(LocationsManager lm) {
        m_locations = lm;
    }

    public void setConferenceBridgeContext(ConferenceBridgeContext conference) {
        m_context = conference;
    }
    
    public void setConferenceBridgeContextImpl(ConferenceBridgeContextImpl impl) {
        m_contextImpl = impl;
    }

    protected void onSetUpInTransaction() throws Exception {
        sql("conference/seed-participants.sql");
        sql("domain/DomainSeed.sql");
    }

    public void testGetBridges() throws Exception {
        assertEquals(2, m_context.getBridges().size());
    }

    public void testGetBridgeByServer() throws Exception {
        assertNull(m_context.getBridgeByServer("uknown"));
        assertEquals("host.example.com", m_context.getBridgeByServer("host.example.com").getName());
    }

    public void testStore() throws Exception {
        Location location = m_locations.getLocation(1000);

        db().execute("delete from meetme_conference");
        db().execute("delete from meetme_bridge");

        Bridge bridge = new Bridge();
        Conference conference = new Conference();
        conference.setName("c1");
        bridge.setLocation(location);
        bridge.addConference(conference);

        m_context.saveBridge(bridge);
        flush();

        assertEquals(1, countRowsInTable("meetme_bridge"));
        assertEquals(1, countRowsInTable("meetme_conference"));
    }

    public void testRemoveConferences() throws Exception {
        m_context.removeConferences(Collections.singleton(new Integer(3002)));
        flush();
        assertEquals(2, countRowsInTable("meetme_bridge"));
        assertEquals(4, countRowsInTable("meetme_conference"));
    }

    public void testLoadBridge() throws Exception {
        Bridge bridge = m_context.loadBridge(new Integer(2006));
        assertEquals(3, bridge.getConferences().size());
    }

    public void testLoadConference() throws Exception {
        Conference conference = m_context.loadConference(new Integer(3001));
        assertEquals("conf_name_3001", conference.getName());
    }

    public void testGetAllConferences() throws Exception {
        List<Conference> conferences = m_context.getAllConferences();

        assertEquals(5, conferences.size());
        Set<String> names = new HashSet<String>();
        for (Conference conference : conferences) {
            names.add(conference.getName());
        }
        assertTrue(names.contains("conf_name_3001"));
        assertTrue(names.contains("conf_name_3002"));
        assertTrue(names.contains("conf_name_3003"));
    }

    public void testFindConferenceByName() throws Exception {
        Conference conference = m_context.findConferenceByName("conf_name_3001");
        assertEquals("conf_name_3001", conference.getName());
    }

    public void testFindConferenceByOwner() throws Exception {
        User owner = m_core.loadUser(1002);
        List<Conference> ownerConferences = m_context.findConferencesByOwner(owner);
        assertEquals(3, ownerConferences.size());
        for (Conference conference : ownerConferences) {
            assertEquals(owner.getId(), conference.getOwner().getId());
        }
    }

    public void testIsAliasInUse() throws Exception {
        // conference names are aliases
        assertTrue(m_context.isAliasInUse("conf_name_3001"));
        assertTrue(m_context.isAliasInUse("conf_name_3002"));
        assertTrue(m_context.isAliasInUse("conf_name_3003"));

        // conference extensions are aliases
        assertTrue(m_context.isAliasInUse("1699"));
        assertTrue(m_context.isAliasInUse("1700"));
        assertTrue(m_context.isAliasInUse("1701"));

        // conference did are aliases
        assertTrue(m_context.isAliasInUse("123456789"));

        // we're not using this extension
        assertFalse(m_context.isAliasInUse("1702"));
    }

    public void testGetBeanIdsOfObjectsWithAlias() throws Exception {
        // conference names are aliases
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("conf_name_3001").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("conf_name_3002").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("conf_name_3003").size() == 1);

        // conference extensions are aliases
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("1699").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("1700").size() == 1);
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("1701").size() == 1);

        // conference did are aliases
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("123456789").size() == 1);

        // we're not using this extension
        assertTrue(m_context.getBeanIdsOfObjectsWithAlias("1702").size() == 0);
    }

    public void testValidate() throws Exception {
        // create a conference with a duplicate extension, should fail to validate
        Conference conf = new Conference();
        conf.setModelFilesContext(TestHelper.getModelFilesContext());
        conf.setName("Appalachian");
        conf.setExtension("1699");
        try {
            // disableDaoEventPublishing();
            m_context.validate(conf);
            fail("conference has duplicate extension but was validated anyway");
        } catch (UserException e) {
            // expected
        }

        conf.setName("SomeConference");
        conf.setExtension(DummyAliasConflicter.MY_ALIAS);
        try {
            m_context.validate(conf);
            fail("extension used by an acd line");
        } catch (UserException e) {
            // expected
        }

        // pick an unused extension, should be OK
        conf.setExtension("1800");
        m_context.validate(conf);

        conf.setSettingValue(Conference.MODERATOR_CODE, "1234");
        conf.setSettingValue(Conference.PARTICIPANT_CODE, null);
        try {
            m_context.validate(conf);
            fail("if we set mod pin should set part pin also");
        } catch (UserException e) {
            // expected
        }

        conf.setSettingValue(Conference.MODERATOR_CODE, null);
        conf.setSettingValue(Conference.PARTICIPANT_CODE, "1234");
        conf.setSettingTypedValue(Conference.QUICKSTART, false);
        try {
            m_context.validate(conf);
            fail("qs conf without moderator pin");
        } catch (UserException e) {
            // expected
        }

        conf.setSettingValue(Conference.MODERATOR_CODE, "1234");
        conf.setSettingValue(Conference.PARTICIPANT_CODE, "1234");
        conf.setSettingTypedValue(Conference.QUICKSTART, false);
        try {
            m_context.validate(conf);
            fail("mod and participant pin same");
        } catch (UserException e) {
            // expected
        }

        conf.setSettingValue(Conference.MODERATOR_CODE, null);
        conf.setSettingValue(Conference.PARTICIPANT_CODE, null);
        conf.setSettingTypedValue(Conference.QUICKSTART, true);
        m_context.validate(conf);

        conf.setSettingValue(Conference.MODERATOR_CODE, "1234");
        conf.setSettingValue(Conference.PARTICIPANT_CODE, "1235");
        conf.setSettingTypedValue(Conference.QUICKSTART, false);
        m_context.validate(conf);

        conf.setExtension("2000");
        conf.setDid("2000");
        try {
            m_context.validate(conf);
            fail();
        } catch (SameExtensionException e) {

        }
    }

    public void testSearchConferences() throws Exception {
        assertEquals(m_message, 1, getConferencesCount("1699"));// ext
        assertEquals(m_message, 3, getConferencesCount("test1002"));// owner username
        assertEquals(m_message, 1, getConferencesCount("test1003"));// owner username
        assertEquals(m_message, 0, getConferencesCount("100"));// owner username - no partial
                                                               // match
        assertEquals(m_message, 0, getConferencesCount("17"));// extension - no partial match
        assertEquals(m_message, 3, getConferencesCount("cOnF_nAmE"));// conf name - partial and
                                                                     // case
        // insensitive match
        assertEquals(m_message, 3, getConferencesCount("MaX"));// owner first name - partial and
                                                               // c.i. match
        assertEquals(m_message, 3, getConferencesCount("AfInO"));// owner last name - partial and
                                                                 // c.i. match
        assertEquals(m_message, 3, getConferencesCount("Maxim Afinogenov"));// owner first+last
                                                                            // name
        assertEquals(m_message, 3, getConferencesCount("AfInOgenov maxim"));// owner last+first
                                                                            // name
        assertEquals(m_message, 1, getConferencesCount("Ilya"));
        assertEquals(m_message, 1, getConferencesCount("conference_no_owner"));
    }

    //remove conf when owner is deleted
    public void testRemoveConferencesByOwner() {
        assertEquals(m_message, 1, getConferencesCount("conf_name_3001"));
        getDaoEventPublisher().resetListeners();
        getDaoEventPublisher().divertEvents(m_contextImpl);
        m_core.deleteUsers(Collections.singletonList(1002));
        assertEquals(m_message, 0, getConferencesCount("conf_name_3001"));
        assertEquals(m_message, 0, getConferencesCount("conf_name_3002"));
        assertEquals(m_message, 0, getConferencesCount("conf_name_3003"));
        assertEquals(m_message, 0, getConferencesCount("conf_name_3004"));
    }
    
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    private String m_message;

    private int getConferencesCount(String searchTerm) {
        List<Conference> confList = m_context.searchConferences(searchTerm);
        m_message = DataCollectionUtil.toString(confList);
        return confList.size();
    }
}
