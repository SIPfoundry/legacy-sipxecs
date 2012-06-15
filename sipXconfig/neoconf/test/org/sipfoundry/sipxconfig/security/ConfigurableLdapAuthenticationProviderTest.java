/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.security;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;

public class ConfigurableLdapAuthenticationProviderTest extends TestCase {

    public void testNewProvider() {
        LdapManager lm = createMock(LdapManager.class);
        ConfigurableLdapAuthenticationProvider p = new ConfigurableLdapAuthenticationProvider();
        p.setLdapManager(lm);
        LdapConnectionParams params = new LdapConnectionParams();
        params.setHost("ldap.example.com");
        // default port 389 is going to be used
        params.setPrincipal("CN=Administrator,CN=Users,DC=corp,DC=exmaple,DC=com");
        params.setSecret("b1rdg33k");
        params.setUseTls(false);
        AttrMap attr = new AttrMap();
        lm.getAttrMap(1);
        expectLastCall().andReturn(attr);
        lm.getConnectionParams(1);
        expectLastCall().andReturn(params);

        replay(lm);
        p.createProvider(1);
        verify(lm);
    }

    public void testProviderForAnonymousAccess() {
        LdapManager lm = createMock(LdapManager.class);
        ConfigurableLdapAuthenticationProvider p = new ConfigurableLdapAuthenticationProvider();
        p.setLdapManager(lm);
        LdapConnectionParams params = new LdapConnectionParams();
        params.setHost("ldap.example.com");
        // default port 389 is going to be used
        params.setPrincipal(null);
        params.setUseTls(false);
        AttrMap attr = new AttrMap();
        lm.getAttrMap(1);
        expectLastCall().andReturn(attr);
        lm.getConnectionParams(1);
        expectLastCall().andReturn(params);

        replay(lm);
        p.createProvider(1);
        verify(lm);
    }

}
