/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Collection;
import java.util.Map;

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.TestHelper.TestCaseDb;
import org.sipfoundry.sipxconfig.admin.CronSchedule;
import org.springframework.context.ApplicationContext;

public class LdapManagerTestDb extends TestCaseDb {

    private LdapManager m_context;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (LdapManager) appContext.getBean(LdapManager.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testLdapSystemSettings() throws Exception {
        LdapSystemSettings settings = m_context.getSystemSettings();    	
        assertNotNull(settings);
        ITable before = TestHelper.getConnection().createDataSet().getTable("ldap_settings");
        assertEquals(0, before.getRowCount());
        settings.setEnableWebAuthentication(true);
        settings.setEnableOpenfireConfiguration(true);
        m_context.saveSystemSettings(settings);
        ITable after = TestHelper.getConnection().createDataSet().getTable("ldap_settings");
        assertEquals(1, after.getRowCount());     
        assertTrue((Boolean)after.getValue(0, "enable_web_authentication"));        
        assertTrue((Boolean)after.getValue(0, "enable_openfire_configuration"));        
    }

    public void testGetConnectionParams() throws Exception {
        LdapConnectionParams connectionParams = m_context.getConnectionParams();
        assertNotNull(connectionParams);
        assertNotNull(connectionParams.getSchedule());

        ITable ldapConnectionTable = TestHelper.getConnection().createDataSet().getTable("ldap_connection");
        assertEquals(1, ldapConnectionTable.getRowCount());

        ITable cronScheduleTable = TestHelper.getConnection().createDataSet().getTable("cron_schedule");
        // this will change once we start keeping something else in the table
        assertEquals(1, cronScheduleTable.getRowCount());
    }

    public void testSetConnectionParams() throws Exception {
        LdapConnectionParams params = new LdapConnectionParams();
        params.setHost("abc");
        params.setPort(1234);
        params.setPrincipal("principal");
        params.setSecret("secret");

        m_context.setConnectionParams(params);
        ITable ldapConnectionTable = TestHelper.getConnection().createDataSet().getTable("ldap_connection");
        assertEquals(1, ldapConnectionTable.getRowCount());

        assertEquals("secret", ldapConnectionTable.getValue(0, "secret"));
        assertEquals(1234, ldapConnectionTable.getValue(0, "port"));
        assertEquals("principal", ldapConnectionTable.getValue(0, "principal"));
        assertEquals("abc", ldapConnectionTable.getValue(0, "host"));
    }

    public void testGetAttrMap() throws Exception {
        AttrMap attrMap = m_context.getAttrMap();
        assertNotNull(attrMap);

        ITable attrMapTable = TestHelper.getConnection().createDataSet().getTable("ldap_attr_map");
        assertEquals(1, attrMapTable.getRowCount());

        ITable userToLdapTable = TestHelper.getConnection().createDataSet().getTable(
                "ldap_user_property_to_ldap_attr");
        assertTrue(1 < userToLdapTable.getRowCount());

        TestHelper.cleanInsertFlat("bulk/ldap/ldap_attr_map.db.xml");
        attrMap = m_context.getAttrMap();
        Map<String, String> userToLdap = attrMap.getUserToLdap();
        assertEquals(2, userToLdap.size());

        assertEquals("uid", attrMap.getAttribute("username"));
        assertEquals("cn", attrMap.getAttribute("firstname"));

        assertEquals("filter", attrMap.getFilter());
        Collection<String> selectedObjectClasses = attrMap.getSelectedObjectClasses();

        assertEquals(2, selectedObjectClasses.size());
        assertTrue(selectedObjectClasses.contains("abc"));
        assertTrue(selectedObjectClasses.contains("def"));
    }
    
    public void testSetAttrMap() throws Exception {
        AttrMap attrMap = m_context.getAttrMap();
        attrMap.setFilter("ou=marketing");
        assertNotNull(attrMap);

        attrMap.setAttribute("a", "1");
        attrMap.setAttribute("b", "2");

        m_context.setAttrMap(attrMap);

        ITable attrMapTable = TestHelper.getConnection().createDataSet().getTable("ldap_attr_map");
        assertEquals(1, attrMapTable.getRowCount());
        assertEquals("ou=marketing", attrMapTable.getValue(0, "filter"));

        ITable userToLdapTable = TestHelper.getConnection().createDataSet().getTable(
                "ldap_user_property_to_ldap_attr");
        assertTrue(1 < userToLdapTable.getRowCount());
    }

    public void testSetSchedule() throws Exception {
        CronSchedule schedule = new CronSchedule();
        schedule.setType(CronSchedule.Type.HOURLY);
        schedule.setMin(15);

        m_context.setSchedule(schedule);

        assertEquals(1, TestHelper.getConnection().getRowCount("cron_schedule",
                "where cron_string = '0 15 * ? * *'"));
    }

    public void testSetScheduleEnabled() throws Exception {
        CronSchedule schedule = new CronSchedule();
        schedule.setType(CronSchedule.Type.WEEKLY);
        schedule.setDayOfWeek(4);
        schedule.setMin(15);
        schedule.setEnabled(true);

        m_context.setSchedule(schedule);

        assertEquals(1, TestHelper.getConnection().getRowCount("cron_schedule",
                "where cron_string = '0 15 0 ? * 4' and enabled = 'true'"));
    }

    public void testGetSetSchedule() throws Exception {
        CronSchedule schedule = m_context.getSchedule();
        m_context.setSchedule(schedule);
    }
}
