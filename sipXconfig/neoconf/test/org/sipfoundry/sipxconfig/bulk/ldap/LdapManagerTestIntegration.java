package org.sipfoundry.sipxconfig.bulk.ldap;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class LdapManagerTestIntegration extends IntegrationTestCase {
    private LdapManager m_ldapManager;

    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    public void testSetConnectionParamsDefaultPort() throws Exception {
        LdapConnectionParams params = new LdapConnectionParams();
        params.setHost("abc");
        params.setPrincipal("principal");
        params.setSecret("secret");

        m_ldapManager.setConnectionParams(params);
        LdapConnectionParams connParams = m_ldapManager.getConnectionParams();

        assertEquals("secret", connParams.getSecret());
        assertNull(connParams.getPort());
        assertEquals("principal", connParams.getPrincipal());
        assertEquals("abc", connParams.getHost());
        assertEquals("ldap://abc:389", connParams.getUrl());
        params.setUseTls(true);
        assertEquals("ldaps://abc:636", connParams.getUrl());
    }
}