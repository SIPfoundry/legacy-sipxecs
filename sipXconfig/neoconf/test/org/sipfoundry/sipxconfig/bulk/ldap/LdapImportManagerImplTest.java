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

public class LdapImportManagerImplTest extends TestCase {

    @SuppressWarnings("unused")
    private LdapImportManager m_ldapImportManager;

    public void testInsert() {
      // FIXME: commented out until we mock LDAP
      // ApplicationContext ac = TestHelper.getApplicationContext();
      // m_ldapImportManager = (LdapImportManager) ac.getBean("ldapImportManagerImpl",
      // LdapImportManager.class);
      // assertNotNull(m_ldapImportManager);
      // m_ldapImportManager.insert();
    }
}
