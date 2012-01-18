/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.List;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class BeanWithSettingsDaoTestIntegration extends IntegrationTestCase {
    private static final String PASSENGER_PIGEON = "pigeon/passenger";
    private BeanWithSettingsDao<BirdSettings> m_dao;
    
    public void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    public void testCreateOne() {
        BirdSettings settings = m_dao.findOrCreateOne();
        assertEquals("0", settings.getSettingValue(PASSENGER_PIGEON));
    }

    public void testFindOne() {
        db().execute("insert into bean_with_settings values (1, 'birdSettings')");
        assertNotNull(m_dao);
        BirdSettings settings = m_dao.findOrCreateOne();
        assertEquals(1, (int) settings.getId());
        assertNotNull(settings.getModelFilesContext());
    }
    
    public void testFindAll() {
        db().execute("insert into bean_with_settings values (1, 'birdSettings'), (2, 'birdSettings')");        
        assertNotNull(m_dao);
        commit();
        List<BirdSettings> settings = m_dao.findAll();
        assertEquals(2, settings.size());
    }
    
    public void testSave() {
        BirdSettings settings = new BirdSettings();
        m_dao.upsert(settings);
        flush();
        db().queryForLong("select 1 from bean_with_settings where bean_id = 'birdSettings'");
    }

    public void testSaveWithSettings() {
        BirdSettings settings = new BirdSettings();
        settings.setSettingValue(PASSENGER_PIGEON, "1");
        m_dao.upsert(settings);
        flush();
        db().queryForLong("select 1 from setting_value where value = '1' and path = ?", PASSENGER_PIGEON);
    }    

    public void setBirdSettingsDao(BeanWithSettingsDao<BirdSettings> birdSettingsDao) {
        m_dao = birdSettingsDao;        
    }
}
