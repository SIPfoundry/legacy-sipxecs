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

import junit.framework.TestCase;

public class AttrMapTest extends TestCase {

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
