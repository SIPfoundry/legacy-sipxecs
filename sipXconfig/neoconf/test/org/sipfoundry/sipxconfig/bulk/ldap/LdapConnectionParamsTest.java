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

import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;
import org.springframework.ldap.support.LdapContextSource;

public class LdapConnectionParamsTest extends TestCase {

    public void testAuthentication() {
        IMocksControl configCtrl = EasyMock.createControl();
        LdapContextSource config = configCtrl.createMock(LdapContextSource.class);
        config.setUrl("ldap://example.sipfoundry.org:10");
        config.setUserName("uid=bongo,dc=sipfoundry,dc=com");
        config.setPassword("abc");
        configCtrl.replay();

        LdapConnectionParams params = new LdapConnectionParams();
        params.setPort(10);
        params.setHost("example.sipfoundry.org");
        params.setPrincipal("uid=bongo,dc=sipfoundry,dc=com");
        params.setSecret("abc");

        params.applyToContext(config);

        configCtrl.verify();
    }
}
