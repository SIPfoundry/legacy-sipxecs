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

import javax.naming.Context;

import junit.framework.TestCase;

public class JndiLdapTemplateTest extends TestCase {

    public void testSetSecurityAuthentication() {
        JndiLdapTemplate template = new JndiLdapTemplate();
        template.setSecurityAuthentication("none");
        assertEquals("none", template.getEnvironment().getProperty(Context.SECURITY_AUTHENTICATION));
        template.setSecurityAuthentication(null);
        assertFalse(template.getEnvironment().containsKey(Context.SECURITY_AUTHENTICATION));
    }
}
