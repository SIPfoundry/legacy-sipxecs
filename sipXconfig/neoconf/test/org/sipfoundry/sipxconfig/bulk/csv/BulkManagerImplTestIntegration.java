/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class BulkManagerImplTestIntegration extends IntegrationTestCase {
    private BulkManager m_bulkManager;
    private SettingDao m_settingDao;
    private PhoneContext m_phoneContext;

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("domain/DomainSeed.sql");
        sql("commserver/SeedLocations.sql");
    }

    public void testInsertFromCsvEmpty() throws Exception {
        m_bulkManager.insertFromCsv(new StringReader(""), false);
        assertEquals(0, countRowsInTable("users"));
        assertEquals(0, countRowsInTable("phone"));
        assertEquals(0, countRowsInTable("line"));
        assertEquals(0, countRowsInTable("user_group"));
        assertEquals(0, countRowsInTable("phone_group"));
    }

    public void testInsertFromCsvNameDuplication() throws Exception {
        // users with duplicated names should be overwritten
        InputStream cutsheet = getClass().getResourceAsStream("dup_names.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet), false);
        commit();
        assertEquals(2, countRowsInTable("users"));
        assertEquals(3, countRowsInTable("phone"));
        assertEquals(3, countRowsInTable("line"));
        assertEquals(2, countRowsInTable("user_group"));
        assertEquals(3, countRowsInTable("phone_group"));
        assertEquals(1, db().queryForLong("select count(*) from group_storage where resource = 'phone'"));
        assertEquals(2, db().queryForLong("select count(*) from group_storage where resource = 'user'"));
    }

    public void testInsertFromCsvAliasDuplication() throws Exception {
        getProfilesDb().dropCollection("userProfile");
        // second user has a duplicated alias - it should be ignored, but remaining users have to
        // be imported
        InputStream cutsheet = getClass().getResourceAsStream("errors.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet), false);
        //commit();
        assertEquals(2, countRowsInTable("users"));
        assertEquals(2, countRowsInTable("phone"));
        assertEquals(2, countRowsInTable("line"));
        assertEquals(2, countRowsInTable("user_group"));
        assertEquals(2, countRowsInTable("phone_group"));
        assertEquals(1, db().queryForLong("select count(*) from group_storage where resource = 'phone'"));
        assertEquals(2, db().queryForLong("select count(*) from group_storage where resource = 'user'"));
    }

    public void testInsertFromCsvPhoneDuplication() throws Exception {
        // users with duplicated names should be overwritten
        InputStream cutsheet = getClass().getResourceAsStream("dup_phones.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet), false);
        assertEquals(5, countRowsInTable("users"));
        assertEquals(4, countRowsInTable("phone"));
        assertEquals(5, countRowsInTable("line"));
        assertEquals(5, countRowsInTable("user_group"));
        assertEquals(4, countRowsInTable("phone_group"));
        assertEquals(2, db().queryForLong("select count(*) from group_storage where resource = 'phone'"));
        assertEquals(2, db().queryForLong("select count(*) from group_storage where resource = 'user'"));
    }

    public void testInsertFromCsv() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("cutsheet.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet), false);
        assertEquals(5, countRowsInTable("users"));
        assertEquals(5, countRowsInTable("phone"));
        assertEquals(5, countRowsInTable("line"));
        assertEquals(5, countRowsInTable("user_group"));
        assertEquals(5, countRowsInTable("phone_group"));
        assertEquals(2, db().queryForLong("select count(*) from group_storage where resource = 'phone'"));
        assertEquals(2, db().queryForLong("select count(*) from group_storage where resource = 'user'"));
    }

    public void testInsertFromCsvDuplicate() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("cutsheet.csv");
        cutsheet.mark(-1);
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet), false);
        // and try again
        cutsheet.reset();
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet), false);
        assertEquals(5, countRowsInTable("users"));
        assertEquals(5, countRowsInTable("phone"));
        // lines are updated, if this value is 10, lines are erroneously being duplicated
        assertEquals(5, countRowsInTable("line"));
        assertEquals(5, countRowsInTable("user_group"));
        assertEquals(5, countRowsInTable("phone_group"));
        assertEquals(2, db().queryForLong("select count(*) from group_storage where resource = 'phone'"));
        assertEquals(2, db().queryForLong("select count(*) from group_storage where resource = 'user'"));
    }

    public void testInsertFromCsvUserNameAliasConflict() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("user_alias_conflict.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet), false);
        assertEquals(1, countRowsInTable("users"));
    }

    public void testInsertFromCsvBlankPhoneGroup() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("blank_phonegroup.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet), false);
        assertEquals(1, countRowsInTable("users"));
        assertEquals(1, countRowsInTable("phone"));
        assertEquals(0, countRowsInTable("phone_group"));
        assertEquals(1, countRowsInTable("user_group"));
    }

    public void testInsertFromCsvBlankUserGroup() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("blank_usergroup.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet), false);
        assertEquals(1, countRowsInTable("users"));
        assertEquals(1, countRowsInTable("phone"));
        assertEquals(1, countRowsInTable("phone_group"));
        assertEquals(0, countRowsInTable("user_group"));
    }

    /*
     * Unfortunately this test can't pass since polycom models
     * are not loaded.
     * Consider moving to polycom project, after enabling integration tests from plugins
     */
    public void _testImportPhoneInheritGroupVersion() throws Exception {
        Group g = new Group();
        g.setName("all");
        g.setSettingValue("group.version/firmware.version", "4.1.X");
        m_settingDao.saveGroup(g);
        InputStream cutsheet = getClass().getResourceAsStream("polycoms_with_groups.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet), false);
        
        List<Phone> phones = m_phoneContext.loadPhones();
        for (Phone phone : phones) {
            if (phone.getModelId().equals("polycom560")) {
                assertEquals("polycom4.0.X", phone.getDeviceVersion().getName());
            }
            else if (phone.getModelId().equals("polycomVVX500")) {
                assertEquals("polycom4.1.X", phone.getDeviceVersion().getName());
            }
        }
    }
    
    public void setBulkManagerImpl(BulkManager bulkManager) {
        m_bulkManager = bulkManager;
    }
}
