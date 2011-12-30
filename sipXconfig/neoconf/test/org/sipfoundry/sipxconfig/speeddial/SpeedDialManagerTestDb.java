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


import java.util.Collections;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class SpeedDialManagerTestDb extends IntegrationTestCase {
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
        assertEquals(buttonCount, countRowsInTable("select count(*) from speeddial_button " +
                " WHERE label = 'testSave'"));
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
        assertEquals(buttonCount, db().queryForLong("select count(*) from speeddial_group_button WHERE label = 'testSave'"));
    }

    public void testOnDeleteUser() throws Exception {
        loadDataSet("speeddial/speeddial.db.xml");
        assertEquals(3, countRowsInTable("speeddial_button"));
        assertEquals(1, countRowsInTable("speeddial"));
        m_coreContext.deleteUsers(Collections.singleton(1001));
        assertEquals(0, countRowsInTable("speeddial_button"));
        assertEquals(0, countRowsInTable("speeddial"));
    }

    public void testOnDeleteGroup() throws Exception {
        loadDataSet("speeddial/speeddial_group.db.xml");
        assertEquals(3, countRowsInTable("speeddial_group_button"));
        assertEquals(1, countRowsInTable("speeddial_group"));
        m_settingDao.deleteGroups(Collections.singleton(1001));
        assertEquals(0, countRowsInTable("speeddial_group_button"));
        assertEquals(0, countRowsInTable("speeddial_group"));
    }

    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

}
