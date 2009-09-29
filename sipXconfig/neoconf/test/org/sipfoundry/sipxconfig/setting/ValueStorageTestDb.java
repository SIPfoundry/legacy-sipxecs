/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingTest.BirdWithSettings;
import org.springframework.context.ApplicationContext;

public class ValueStorageTestDb extends SipxDatabaseTestCase {

    private SettingDao m_dao;
    private BeanWithSettings m_bean;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_dao = (SettingDao) context.getBean("settingDao");
        m_bean = new BirdWithSettings();
    }

    public void testSave() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");

        m_bean.setSettingValue("towhee/canyon", "chirp");
        m_bean.setSettingValue("pigeon/rock-dove", null);

        ValueStorage vs = (ValueStorage) m_bean.getValueStorage();
        m_dao.storeValueStorage(vs);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/SaveValueStorageExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[value_storage_id]", vs.getId());

        ITable expected = expectedRds.getTable("setting_value");

        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting_value");

        Assertion.assertEquals(expected, actual);
    }

    public void testUpdate() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("setting/UpdateValueStorageSeed.xml");

        ValueStorage vs = m_dao.loadValueStorage(new Integer(1));
        m_bean.setValueStorage(vs);

        m_bean.setSettingValue("towhee/canyon", null);
        m_bean.setSettingValue("pigeon/rock-dove", "coo coo");

        m_dao.storeValueStorage(vs);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/UpdateValueStorageExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        ITable expected = expectedRds.getTable("setting_value");

        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting_value");

        Assertion.assertEquals(expected, actual);
    }
}
