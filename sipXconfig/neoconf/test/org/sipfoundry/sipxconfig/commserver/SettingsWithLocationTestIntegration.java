/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver;

import java.io.IOException;
import java.util.List;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class SettingsWithLocationTestIntegration extends IntegrationTestCase {
    private static final String PASSENGER_PIGEON = "pigeon/passenger";
    private SettingsWithLocationDao<BirdWithLocation> m_dao;
    private LocationsManager m_locationsManager;
    
    public void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    public void testCreateOne() {
        Location location = TestHelper.createDefaultLocation();
        BirdWithLocation settings = m_dao.findOrCreate(location);
        assertEquals("0", settings.getSettingValue(PASSENGER_PIGEON));        
    }

    public void testFindOne() throws IOException {
        sql("commserver/SeedLocations.sql");
        sql("commserver/SeedSettingsWithLocation.sql");
        Location location = new Location();
        location.setUniqueId(101);
        BirdWithLocation settings = m_dao.findOrCreate(location);
        assertEquals(1, (int) settings.getId());
        assertEquals(location.getId(), settings.getLocation().getId());
        assertNotNull(settings.getModelFilesContext());
    }
    
    public void testFindAll() throws IOException {
        sql("commserver/SeedLocations.sql");
        sql("commserver/SeedSettingsWithLocation.sql");
        Location location = new Location();
        location.setUniqueId(101);
        List<BirdWithLocation> settings = m_dao.findAll();
        assertEquals(2, settings.size());
    }
    
    public void testSave() throws IOException {
        sql("commserver/SeedLocations.sql");
        Location location = m_locationsManager.getLocation(101);
        BirdWithLocation settings = new BirdWithLocation();
        settings.setLocation(location);
        m_dao.upsert(settings);
        flush();
        db().queryForLong("select 1 from settings_with_location where bean_id = 'birdWithLocation'");
    }

    public void testSaveWithSettings() throws IOException {
        sql("commserver/SeedLocations.sql");
        BirdWithLocation settings = new BirdWithLocation();
        Location location = m_locationsManager.getLocation(101);
        settings.setLocation(location);
        settings.setSettingValue(PASSENGER_PIGEON, "1");
        m_dao.upsert(settings);
        flush();
        db().queryForLong("select 1 from setting_value where value = '1' and path = ?", PASSENGER_PIGEON);
    }    

    public void setBirdWithLocationDao(SettingsWithLocationDao<BirdWithLocation> dao) {
        m_dao = dao;        
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
