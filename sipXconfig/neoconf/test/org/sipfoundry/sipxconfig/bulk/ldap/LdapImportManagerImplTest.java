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

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class LdapImportManagerImplTest extends TestCase {

    private LdapImportManager m_ldapImportManager;

    protected void setUp() throws Exception {
        ApplicationContext ac = TestHelper.getApplicationContext();
        m_ldapImportManager = (LdapImportManager) ac.getBean("ldapImportManagerImpl",
                LdapImportManager.class);
    }

    public void testInsert() {
        assertNotNull(m_ldapImportManager);
        // FIXME: commented out until we mock LDAP
        // m_ldapImportManager.insert();
    }

}
