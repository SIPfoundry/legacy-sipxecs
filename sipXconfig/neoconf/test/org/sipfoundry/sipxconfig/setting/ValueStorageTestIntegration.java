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

import static org.junit.Assert.assertArrayEquals;

import org.sipfoundry.sipxconfig.setting.BeanWithSettingTest.BirdWithSettings;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.ResultDataGrid;

public class ValueStorageTestIntegration extends IntegrationTestCase {
    private SettingDao m_settingDao;
    private BeanWithSettings m_bean;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        clear();
        m_bean = new BirdWithSettings();
    }

    public void testSave() throws Exception {
        m_bean.setSettingValue("towhee/canyon", "chirp");
        m_bean.setSettingValue("pigeon/rock-dove", null);

        ValueStorage vs = (ValueStorage) m_bean.getValueStorage();
        m_settingDao.storeValueStorage(vs);
        commit();
        Object[][] expected = new Object[][] {
                { "", "pigeon/rock-dove" },
                { "chirp", "towhee/canyon" }
        };
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select value, path from value_storage as v, setting_value as s where v.value_storage_id = s.value_storage_id order by s.value", actual);
        assertArrayEquals(expected, actual.toArray());
    }

    public void testUpdate() throws Exception {
        loadDataSet("setting/UpdateValueStorageSeed.xml");

        ValueStorage vs = m_settingDao.loadValueStorage(new Integer(1));
        m_bean.setValueStorage(vs);

        m_bean.setSettingValue("towhee/canyon", null);
        m_bean.setSettingValue("pigeon/rock-dove", "coo coo");

        m_settingDao.storeValueStorage(vs);
        commit();
        Object[][] expected = new Object[][] {
                { "coo coo", "pigeon/rock-dove" }
        };
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select value, path from value_storage as v, setting_value as s where v.value_storage_id = s.value_storage_id", actual);
        assertArrayEquals(expected, actual.toArray());
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }
}
