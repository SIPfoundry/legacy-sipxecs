/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.api;

import junit.framework.TestCase;

public class UserBuilderTest extends TestCase {    
    private UserBuilder m_builder;
    private org.sipfoundry.sipxconfig.common.User m_myUser;
    private User m_apiUser;
    
    protected void setUp() {
        m_builder = new UserBuilder();
        m_myUser = new org.sipfoundry.sipxconfig.common.User();
        m_apiUser = new User();
    }

    public void testFromApi() {        
        m_apiUser.setAliases(new String[] {"one" , "two"});
        ApiBeanUtil.toMyObject(m_builder, m_myUser, m_apiUser);
        assertEquals("one two", m_myUser.getAliasesString());
    }

    public void testToApi() {
        m_myUser.setAliasesString("one two");
        ApiBeanUtil.toApiObject(m_builder, m_apiUser, m_myUser);
        assertEquals("one", m_apiUser.getAliases(0));
        assertEquals("two", m_apiUser.getAliases(1));
    }
}
