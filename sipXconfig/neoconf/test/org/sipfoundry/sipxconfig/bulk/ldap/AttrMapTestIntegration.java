package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Collection;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class AttrMapTestIntegration extends IntegrationTestCase {

    public void testGetLdapAttributesArray() throws Exception {
        ApplicationContext ac = TestHelper.getApplicationContext();
        AttrMap attrMap = (AttrMap) ac.getBean("attrMap", AttrMap.class);
        String[] ldapAttributesArray = attrMap.getLdapAttributesArray();
        for (String attr : ldapAttributesArray) {
            assertNotNull(attr);
        }
    }

    public void testGetLdapAttibutes() throws Exception {
        ApplicationContext ac = TestHelper.getApplicationContext();
        AttrMap attrMap = (AttrMap) ac.getBean("attrMap", AttrMap.class);
        for (String name : attrMap.getLdapAttributes()) {
            assertNotNull(name);
        }
    }

    public void testIdentityAttribute() {
        ApplicationContext ac = TestHelper.getApplicationContext();
        AttrMap attrMap = (AttrMap) ac.getBean("attrMap", AttrMap.class);
        Collection<String> ldapAttributes = attrMap.getLdapAttributes();
        assertTrue(ldapAttributes.contains(attrMap.getIdentityAttributeName()));
    }
}
