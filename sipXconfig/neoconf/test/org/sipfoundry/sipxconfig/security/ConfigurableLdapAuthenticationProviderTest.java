/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
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
        params.setPort(389);
        params.setPrincipal("CN=Administrator,CN=Users,DC=corp,DC=exmaple,DC=com");
        params.setSecret("b1rdg33k");
        params.setUseTls(false);
        AttrMap attr = new AttrMap();
        lm.getAttrMap();
        expectLastCall().andReturn(attr);        
        lm.getConnectionParams();
        expectLastCall().andReturn(params);

        replay(lm);
        p.createProvider();
        verify(lm);
    }
}
