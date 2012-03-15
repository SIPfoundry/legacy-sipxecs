/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
