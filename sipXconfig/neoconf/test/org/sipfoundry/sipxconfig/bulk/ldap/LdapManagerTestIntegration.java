package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings.AuthenticationOptions;
import org.sipfoundry.sipxconfig.common.CronSchedule;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class LdapManagerTestIntegration extends IntegrationTestCase {
    private LdapManager m_ldapManager;
    private AttrMap m_attrMap;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testLdapSystemSettings() throws Exception {
        LdapSystemSettings settings = m_ldapManager.getSystemSettings();
        assertNotNull(settings);
        assertEquals(0, countRowsInTable("ldap_settings"));
        settings.setAuthenticationOptions(AuthenticationOptions.LDAP);
        settings.setEnableOpenfireConfiguration(true);
        m_ldapManager.saveSystemSettings(settings);
        commit();
        assertEquals(1, countRowsInTable("ldap_settings"));
        Map<String, Object> after = db().queryForMap("select * from ldap_settings");
        assertEquals("LDAP", after.get("authentication_options"));
        assertTrue((Boolean)after.get("enable_openfire_configuration"));
        //by default LDAP configuration is not activated
        assertFalse((Boolean)after.get("configured"));
    }

    public void testNoLdapSystemSettings() throws Exception {
        LdapSystemSettings settings = m_ldapManager.getSystemSettings();
        //test noLDAP / unconfigured
        settings.setAuthenticationOptions(AuthenticationOptions.NO_LDAP);
        settings.setEnableOpenfireConfiguration(true);
        settings.setConfigured(false);
        m_ldapManager.saveSystemSettings(settings);
        commit();
        assertEquals(1, countRowsInTable("ldap_settings"));
        Map<String, Object> after = db().queryForMap("select * from ldap_settings");
        assertEquals("noLDAP", after.get("authentication_options"));
        assertFalse((Boolean)after.get("configured"));
    }

    public void testPinLdapSystemSettings() throws Exception {
        LdapSystemSettings settings = m_ldapManager.getSystemSettings();
        //test pinLDAP
        settings.setAuthenticationOptions(AuthenticationOptions.PIN_LDAP);
        settings.setEnableOpenfireConfiguration(true);
        m_ldapManager.saveSystemSettings(settings);
        commit();
        assertEquals(1, countRowsInTable("ldap_settings"));
        Map<String, Object> after = db().queryForMap("select * from ldap_settings");
        assertEquals("pinLDAP", after.get("authentication_options"));
    }

    public void testGetConnectionParams() throws Exception {
        LdapConnectionParams params1 = new LdapConnectionParams();
        LdapConnectionParams params2 = new LdapConnectionParams();
        m_ldapManager.setConnectionParams(params1);
        m_ldapManager.setConnectionParams(params2);
        LdapConnectionParams connectionParams = m_ldapManager.getAllConnectionParams().get(0);
        assertNotNull(connectionParams);
        assertNotNull(connectionParams.getSchedule());
        commit();
        assertEquals(2, countRowsInTable("ldap_connection"));

        // this will change once we start keeping something else in the table
        assertEquals(2, countRowsInTable("cron_schedule"));
    }

    public void testSetConnectionParams() throws Exception {
        LdapConnectionParams params = new LdapConnectionParams();
        params.setHost("abc");
        params.setPort(1234);
        params.setPrincipal("principal");
        params.setSecret("secret");

        m_ldapManager.setConnectionParams(params);
        commit();
        assertEquals(1,  countRowsInTable("ldap_connection"));
        Map<String, Object> after = db().queryForMap("select * from ldap_connection");

        assertEquals("secret", after.get("secret"));
        assertEquals(1234, after.get("port"));
        assertEquals("principal", after.get("principal"));
        assertEquals("abc", after.get("host"));
    }

    public void testGetAttrMapInit() throws Exception {
        m_attrMap.setUniqueId(1);
        m_ldapManager.setAttrMap(m_attrMap);
        AttrMap attrMap = m_ldapManager.getAttrMap(1);
        assertNotNull(attrMap);
        commit();
        assertEquals(1, countRowsInTable("ldap_attr_map"));
        assertTrue(1 < countRowsInTable("ldap_user_property_to_ldap_attr"));
    }

    public void testGetAttrMap() throws Exception {
        sql("bulk/ldap/ldap_attr_map.sql");
        commit();
        AttrMap attrMap = m_ldapManager.getAttrMap(1000);
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
        AttrMap attrMap = m_ldapManager.getAttrMap(1);
        attrMap.setFilter("ou=marketing");
        assertNotNull(attrMap);

        attrMap.setAttribute("a", "1");
        attrMap.setAttribute("b", "2");

        m_ldapManager.setAttrMap(attrMap);

        commit();
        assertEquals(1, countRowsInTable("ldap_attr_map"));
        assertEquals("ou=marketing", db().queryForObject("select filter from ldap_attr_map", String.class));
        assertTrue(1 < countRowsInTable("ldap_user_property_to_ldap_attr"));
    }

    public void testSetSchedule() throws Exception {
        CronSchedule schedule = new CronSchedule();
        schedule.setType(CronSchedule.Type.HOURLY);
        schedule.setMin(15);
        LdapConnectionParams params = new LdapConnectionParams();
        params.setSchedule(schedule);
        m_ldapManager.setConnectionParams(params);
        m_ldapManager.setSchedule(schedule,params.getId());

        commit();
        assertEquals(1, db().queryForLong("select count(*) from cron_schedule where cron_string = '0 15 * ? * *'"));
    }

    public void testSetScheduleEnabled() throws Exception {
        CronSchedule schedule = new CronSchedule();
        schedule.setType(CronSchedule.Type.WEEKLY);
        schedule.setDayOfWeek(4);
        schedule.setMin(15);
        schedule.setEnabled(true);
        LdapConnectionParams params = new LdapConnectionParams();
        params.setSchedule(schedule);
        m_ldapManager.setConnectionParams(params);
        m_ldapManager.setSchedule(schedule,params.getId());

        commit();
        assertEquals(1, db().queryForLong("select count(*) from cron_schedule where "
                + " cron_string = '0 15 0 ? * 4' and enabled = 'true'"));
    }

    public void testGetSetSchedule() throws Exception {
        CronSchedule schedule = m_ldapManager.getSchedule(1);
        m_ldapManager.setSchedule(schedule,1);
    }

    public void testSetConnectionParamsDefaultPort() throws Exception {
        LdapConnectionParams params = m_ldapManager.getConnectionParams(-1);
        params.setHost("abc");
        params.setPrincipal("principal");
        params.setSecret("secret");

        m_ldapManager.setConnectionParams(params);
        LdapConnectionParams connParams = m_ldapManager.getConnectionParams(params.getId());

        assertEquals("secret", connParams.getSecret());
        assertNull(connParams.getPort());
        assertEquals("principal", connParams.getPrincipal());
        assertEquals("abc", connParams.getHost());
        assertEquals("ldap://abc:389", connParams.getUrl());
        params.setUseTls(true);
        assertEquals("ldaps://abc:636", connParams.getUrl());
    }

    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    public void setAttrMap(AttrMap attrMap) {
        m_attrMap = attrMap;
    }

}