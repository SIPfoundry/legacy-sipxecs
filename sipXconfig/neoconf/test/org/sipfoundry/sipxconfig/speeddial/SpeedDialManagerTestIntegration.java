/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import static org.sipfoundry.sipxconfig.common.AbstractUser.IM_ACCOUNT;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.test.ImdbTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

import com.mongodb.BasicDBObject;
import com.mongodb.DBObject;
import com.mongodb.QueryBuilder;

import edu.emory.mathcs.backport.java.util.Arrays;

public class SpeedDialManagerTestIntegration extends ImdbTestCase {
    private SpeedDialManager m_speedDialManager;
    private CoreContext m_coreContext;
    private SettingDao m_settingDao;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testGetSpeedDialForUser() throws Exception {
        loadDataSet("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_speedDialManager.getSpeedDialForUserId(1001, true);
        assertNotNull(speedDial);
        assertEquals(3, speedDial.getButtons().size());
        assertEquals("222", speedDial.getButtons().get(2).getNumber());
        assertFalse(speedDial.getButtons().get(0).isBlf());
        assertTrue(speedDial.getButtons().get(1).isBlf());
    }

    public void testGetSpeedDialForGroupId() throws Exception {
        loadDataSet("speeddial/speeddial_group.db.xml");
        SpeedDialGroup speedDialGroup = m_speedDialManager.getSpeedDialForGroupId(1001);
        assertNotNull(speedDialGroup);
        assertEquals(3, speedDialGroup.getButtons().size());
        assertEquals("222", speedDialGroup.getButtons().get(2).getNumber());
        assertFalse(speedDialGroup.getButtons().get(0).isBlf());
        assertTrue(speedDialGroup.getButtons().get(1).isBlf());
    }

    public void testGetNewSpeedDialForUser() throws Exception {
        loadDataSet("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_speedDialManager.getSpeedDialForUserId(1002, false);
        assertNull(speedDial);
        speedDial = m_speedDialManager.getSpeedDialForUserId(1002, true);
        assertNotNull(speedDial);
        assertEquals(0, speedDial.getButtons().size());
    }

    public void testSaveSpeedDialForUser() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        loadDataSetXml("commserver/seedLocations.xml");
        loadDataSet("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_speedDialManager.getSpeedDialForUserId(1002, true);

        final int buttonCount = 5;
        for (int i = 0; i < buttonCount; i++) {
            Button b = new Button();
            b.setLabel("testSave");
            b.setNumber(String.valueOf(i));
            b.setBlf(i % 2 == 0);
            speedDial.getButtons().add(b);
        }

        m_speedDialManager.saveSpeedDial(speedDial);
        assertEquals(buttonCount, db().queryForLong(
                "select count(*) from speeddial_button " + " WHERE label = 'testSave'"));

        DBObject user = new BasicDBObject().append(ID, "User1002");
        BasicDBObject speeddial = new BasicDBObject("usr", "~~rl~F~user2").append("usrcns", "~~rl~C~user2");
        List<DBObject> btns = new ArrayList<DBObject>();
        btns.add(new BasicDBObject("uri", "sip:0@example.org").append("name", "testSave"));
        btns.add(new BasicDBObject("uri", "sip:2@example.org").append("name", "testSave"));
        btns.add(new BasicDBObject("uri", "sip:4@example.org").append("name", "testSave"));
        speeddial.append("btn", btns);
        user.put("spdl", speeddial);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user);

        // Test clear speed dials directly from Mongo
        int result = getEntityCollection().find(QueryBuilder.start(MongoConstants.SPEEDDIAL).exists(true).get())
                .count();
        assertEquals(1, result);
        m_speedDialManager.clear();
        result = getEntityCollection().find(QueryBuilder.start(MongoConstants.SPEEDDIAL).exists(true).get()).count();
        assertEquals(0, result);
    }

    public void testVerifyBlf() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        loadDataSetXml("commserver/seedLocations.xml");
        loadDataSet("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_speedDialManager.getSpeedDialForUserId(1002, true);
        Button b = new Button();
        b.setLabel("testSave");
        b.setNumber("test@test.org");
        b.setBlf(true);
        speedDial.getButtons().add(b);
        m_speedDialManager.saveSpeedDial(speedDial);
        Button b1 = new Button();
        b1.setLabel("testSave1");
        b1.setNumber("8888");
        b1.setBlf(true);
        speedDial.getButtons().add(b1);
        try {
            m_speedDialManager.saveSpeedDial(speedDial);
            fail();
        } catch (UserException ex) {
            assertTrue(true);
        }
        b1.setBlf(false);
        m_speedDialManager.saveSpeedDial(speedDial);
    }

    public void testSaveSpeedDialForGroup() throws Exception {
        loadDataSet("speeddial/speeddial_group.db.xml");
        SpeedDialGroup speedDialgroup = m_speedDialManager.getSpeedDialForGroupId(1002);

        final int buttonCount = 5;
        for (int i = 0; i < buttonCount; i++) {
            Button b = new Button();
            b.setLabel("testSave");
            b.setNumber(String.valueOf(i));
            b.setBlf(i % 2 == 0);
            speedDialgroup.getButtons().add(b);
        }

        m_speedDialManager.saveSpeedDialGroup(speedDialgroup);
        assertEquals(buttonCount,
                db().queryForLong("select count(*) from speeddial_group_button WHERE label = 'testSave'"));
        /*
         * here, we should see all users in group updated, but we don't - due to operations
         * happening in different transactions group get members do not return what it should.
         * DBObject user = new BasicDBObject().append(ID, "User4002"); BasicDBObject speeddial =
         * new BasicDBObject("usr", "~~rl~F~user2") .append("usrcns", "~~rl~C~user2");
         * List<DBObject> btns = new ArrayList<DBObject>(); btns.add(new BasicDBObject("uri",
         * "sip:0@example.org").append("name", "testSave")); btns.add(new BasicDBObject("uri",
         * "sip:2@example.org").append("name", "testSave")); btns.add(new BasicDBObject("uri",
         * "sip:4@example.org").append("name", "testSave")); speeddial.append("btn", btns);
         * user.put("spdl", speeddial);
         *
         * MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user);
         */
    }

    public void testSpeedDialSynchToGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        loadDataSetXml("commserver/seedLocations.xml");
        loadDataSet("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_speedDialManager.getSpeedDialForUserId(1003, true);
        assertEquals(3, speedDial.getButtons().size());
        m_speedDialManager.saveSpeedDial(speedDial);
        assertEquals(3, m_speedDialManager.getSpeedDialForUserId(1003, true).getButtons().size());

        final int buttonCount = 5;
        for (int i = 0; i < buttonCount; i++) {
            Button b = new Button();
            b.setLabel("testSave");
            b.setNumber(String.valueOf(i));
            b.setBlf(i % 2 == 0);
            speedDial.getButtons().add(b);
        }

        m_speedDialManager.saveSpeedDial(speedDial);
        assertEquals(8, m_speedDialManager.getSpeedDialForUserId(1003, true).getButtons().size());

        m_speedDialManager.speedDialSynchToGroup(m_coreContext.getUser(1003));
        assertEquals(3, m_speedDialManager.getSpeedDialForUserId(1003, true).getButtons().size());

        DBObject user = new BasicDBObject().append(ID, "User1003");
        BasicDBObject speeddial = new BasicDBObject("usr", "~~rl~F~user3").append("usrcns", "~~rl~C~user3");
        List<DBObject> btns = new ArrayList<DBObject>();
        btns.add(new BasicDBObject("uri", "sip:111@example.org").append("name", "B"));// only one
                                                                                      // subscribe
        speeddial.append("btn", btns);
        user.put("spdl", speeddial);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user);
    }

    public void testOnDeleteUser() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        flush();
        loadDataSetXml("commserver/seedLocations.xml");
        loadDataSet("speeddial/speeddial.db.xml");
        assertEquals(5, countRowsInTable("speeddial_button"));
        assertEquals(2, countRowsInTable("speeddial"));
        getDaoEventPublisher().resetListeners();
        m_coreContext.deleteUsers(Collections.singleton(1001));
        flush();
        assertEquals(2, countRowsInTable("speeddial_button"));
        assertEquals(1, countRowsInTable("speeddial"));
    }

    public void testOnDeleteGroup() throws Exception {
        loadDataSet("speeddial/speeddial_group.db.xml");
        assertEquals(3, countRowsInTable("speeddial_group_button"));
        assertEquals(1, countRowsInTable("speeddial_group"));
        m_settingDao.deleteGroups(Collections.singleton(1001));
        assertEquals(0, countRowsInTable("speeddial_group_button"));
        assertEquals(0, countRowsInTable("speeddial_group"));
    }

    public void testSpeedDialRemovalOnUserDelete() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        loadDataSetXml("commserver/seedLocations.xml");
        loadDataSet("speeddial/speeddial.db.xml");

        User u1020 = m_coreContext.loadUser(1020);
        m_coreContext.saveUser(u1020);

        DBObject user = new BasicDBObject().append(ID, "User1020");
        BasicDBObject speeddial = new BasicDBObject("usr", "~~rl~F~user20").append("usrcns", "~~rl~C~user20");
        List<DBObject> btns = new ArrayList<DBObject>();
        btns.add(new BasicDBObject("uri", "sip:user21@example.org").append("name", "X"));
        btns.add(new BasicDBObject("uri", "sip:user22@example.org").append("name", "X"));
        speeddial.append("btn", btns);
        user.put("spdl", speeddial);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user);

        m_coreContext.deleteUsers(Arrays.asList(new Integer[]{1021}));

        MongoTestCaseHelper.assertObjectNotPresent(getEntityCollection(), user);
    }

    public void testXmppSpecialUser() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        loadDataSetXml("commserver/seedLocations.xml");
        loadDataSet("speeddial/speeddial-special-user.db.xml");

        User u1001 = m_coreContext.loadUser(1001);
        u1001.setSettingTypedValue(IM_ACCOUNT, true);
        m_coreContext.saveUser(u1001);

        DBObject user = new BasicDBObject().append(ID, "~~id~xmpprlsclient");
        BasicDBObject speeddial = new BasicDBObject("usr", "~~rl~F~~~id~xmpprlsclient").append("usrcns",
                "~~rl~C~~~id~xmpprlsclient");
        List<DBObject> btns = new ArrayList<DBObject>();
        btns.add(new BasicDBObject("uri", "sip:user1@example.org").append("name", "user1"));
        speeddial.append("btn", btns);
        user.put("spdl", speeddial);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user);

        u1001.setSettingTypedValue(IM_ACCOUNT, false);
        m_coreContext.saveUser(u1001);

        MongoTestCaseHelper.assertObjectNotPresent(getEntityCollection(), user);

        u1001.setSettingTypedValue(IM_ACCOUNT, true);
        m_coreContext.saveUser(u1001);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user);

        m_coreContext.deleteUser(u1001);

        MongoTestCaseHelper.assertObjectNotPresent(getEntityCollection(), user);

        User u1002 = m_coreContext.loadUser(1002);
        User u1003 = m_coreContext.loadUser(1003);
        User u1004 = m_coreContext.loadUser(1004);

        Group group = m_settingDao.getGroup(1003);
        group.setSettingValue("im/im-account", "1");
        m_settingDao.saveGroup(group);

        btns = new ArrayList<DBObject>();
        btns.add(new BasicDBObject("uri", "sip:user2@example.org").append("name", "user2"));
        btns.add(new BasicDBObject("uri", "sip:user3@example.org").append("name", "user3"));
        speeddial.append("btn", btns);
        user.put("spdl", speeddial);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user);

        m_coreContext.deleteUser(u1002);
        MongoTestCaseHelper.assertObjectNotPresent(getEntityCollection(), user);
        btns = new ArrayList<DBObject>();
        btns.add(new BasicDBObject("uri", "sip:user3@example.org").append("name", "user3"));
        speeddial.append("btn", btns);
        user.put("spdl", speeddial);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user);
        group.setSettingValue("im/im-account", "0");
        m_settingDao.saveGroup(group);

        MongoTestCaseHelper.assertObjectNotPresent(getEntityCollection(), user);

        group.setSettingValue("im/im-account", "1");
        m_settingDao.saveGroup(group);

        btns = new ArrayList<DBObject>();
        btns.add(new BasicDBObject("uri", "sip:user3@example.org").append("name", "user3"));
        speeddial.append("btn", btns);
        user.put("spdl", speeddial);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user);

        Set<Group> groups = new TreeSet<Group>();
        groups.add(group);
        u1004.setGroups(groups);
        m_coreContext.saveUser(u1004);

        btns = new ArrayList<DBObject>();
        btns.add(new BasicDBObject("uri", "sip:user3@example.org").append("name", "user3"));
        btns.add(new BasicDBObject("uri", "sip:user4@example.org").append("name", "user4"));
        speeddial.append("btn", btns);
        user.put("spdl", speeddial);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user);

        m_settingDao.deleteGroups(Arrays.asList(new Integer[] {
            1003
        }));

        MongoTestCaseHelper.assertObjectNotPresent(getEntityCollection(), user);
    }

    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }

    @Override
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

}
