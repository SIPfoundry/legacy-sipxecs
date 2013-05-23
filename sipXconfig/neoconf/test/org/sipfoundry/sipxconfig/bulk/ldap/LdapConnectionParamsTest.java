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

import java.util.HashMap;
import java.util.Map;

import javax.naming.Context;

import junit.framework.TestCase;

import org.easymock.classextension.EasyMock;
import org.springframework.ldap.core.support.LdapContextSource;

public class LdapConnectionParamsTest extends TestCase {

    public void testAuthentication() {
        Map other = new HashMap();
        other.put(Context.REFERRAL, "follow");

        LdapContextSource config = EasyMock.createMock(LdapContextSource.class);
        config.setUrl("ldap://example.sipfoundry.org:10");
        config.setUserDn("uid=bongo,dc=sipfoundry,dc=com");
        config.setPassword("abc");
        config.setBaseEnvironmentProperties(other);

        EasyMock.replay(config);

        LdapConnectionParams params = new LdapConnectionParams();
        params.setPort(10);
        params.setHost("example.sipfoundry.org");
        params.setPrincipal("uid=bongo,dc=sipfoundry,dc=com");
        params.setSecret("abc");
        params.setReferral("follow");

        params.applyToContext(config);

        EasyMock.verify(config);
    }

    public void testAuthenticationDefaultPort() {
        Map other = new HashMap();
        other.put(Context.REFERRAL, "follow");

        LdapContextSource config = EasyMock.createMock(LdapContextSource.class);
        config.setUrl("ldap://example.sipfoundry.org:389");
        config.setUserDn("uid=bongo,dc=sipfoundry,dc=com");
        config.setPassword("abc");
        config.setBaseEnvironmentProperties(other);

        EasyMock.replay(config);

        LdapConnectionParams params = new LdapConnectionParams();
        params.setHost("example.sipfoundry.org");
        params.setPrincipal("uid=bongo,dc=sipfoundry,dc=com");
        params.setSecret("abc");
        params.setReferral("follow");

        params.applyToContext(config);

        EasyMock.verify(config);
    }
}
