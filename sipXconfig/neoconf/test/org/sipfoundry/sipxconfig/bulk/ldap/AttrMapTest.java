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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class AttrMapTest extends TestCase {

    private AttrMap m_map;

    protected void setUp() throws Exception {
        ApplicationContext ac = TestHelper.getApplicationContext();
        m_map = (AttrMap) ac.getBean("attrMap", AttrMap.class);
    }
    
    public void testGetLdapAttributesArray() throws Exception {        
        String[] ldapAttributesArray = m_map.getLdapAttributesArray();        
        for (String attr : ldapAttributesArray) {
            assertNotNull(attr);
        }
    }
    
    public void testGetLdapAttibutes() throws Exception {
        for (String name : m_map.getLdapAttributes()) {
            assertNotNull(name);
        }
    }
    
    public void testIdentityAttribute() {
        Collection<String> ldapAttributes = m_map.getLdapAttributes();
        assertTrue(ldapAttributes.contains(m_map.getIdentityAttributeName()));
    }
    
    public void testGetSearchFilter() {
        AttrMap map = new AttrMap();
        assertEquals("(objectclass=*)", map.getSearchFilter());
        map.setObjectClass("person");
        assertEquals("(objectclass=person)", map.getSearchFilter());        
    }


    public void testGetSearchFilterExtra() {
        AttrMap map = new AttrMap();
        map.setFilter("attr=kuku");
        assertEquals("(&(objectclass=*)(attr=kuku))", map.getSearchFilter());        
        map.setObjectClass("person");
        assertEquals("(&(objectclass=person)(attr=kuku))", map.getSearchFilter());        
        map.setFilter("(attr=kuku)");
        assertEquals("(&(objectclass=person)(attr=kuku))", map.getSearchFilter());        
        map.setFilter("!(attr=kuku)(attr=bongo)");
        assertEquals("(&(objectclass=person)(!(attr=kuku)(attr=bongo)))", map.getSearchFilter());        
        map.setFilter("(!(attr=kuku)(attr=bongo))");
        assertEquals("(&(objectclass=person)(!(attr=kuku)(attr=bongo)))", map.getSearchFilter());        
    }
}
