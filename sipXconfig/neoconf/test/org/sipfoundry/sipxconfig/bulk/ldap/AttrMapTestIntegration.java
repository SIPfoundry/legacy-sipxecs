package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Collection;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class AttrMapTestIntegration extends IntegrationTestCase {
    
    AttrMap m_attrMap;
    
    public void setAttrMap(AttrMap attrMap) {
        m_attrMap = attrMap;
    }

    public void testGetLdapAttributesArray() throws Exception {
        String[] ldapAttributesArray = m_attrMap.getLdapAttributesArray();
        for (String attr : ldapAttributesArray) {
            assertNotNull(attr);
        }
    }

    public void testGetLdapAttibutes() throws Exception {
        for (String name : m_attrMap.getLdapAttributes()) {
            assertNotNull(name);
        }
    }

    public void testIdentityAttribute() {
        Collection<String> ldapAttributes = m_attrMap.getLdapAttributes();
        assertTrue(ldapAttributes.contains(m_attrMap.getIdentityAttributeName()));
    }
}
