/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.api;

import java.util.Arrays;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SystemServiceTest extends TestCase {

    public void testSystemInfo() throws Exception {
        org.sipfoundry.sipxconfig.domain.Domain testDomain = new org.sipfoundry.sipxconfig.domain.Domain(
                "greebe");
        testDomain.addAlias("horned");
        testDomain.addAlias("pie-billed");
        IMocksControl domainManagerControl = EasyMock.createStrictControl();
        DomainManager domainManager = domainManagerControl.createMock(DomainManager.class);
        domainManager.getDomain();
        domainManagerControl.andReturn(testDomain);
        domainManagerControl.replay();

        IMocksControl coreContextControl = EasyMock.createStrictControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        coreContext.getAuthorizationRealm();
        coreContextControl.andReturn("myauthrealm");
        coreContextControl.replay();

        SystemServiceImpl service = new SystemServiceImpl();
        service.setDomainManager(domainManager);
        service.setCoreContext(coreContext);
        SystemInfo info = service.systemInfo();
        assertNotNull(info);
        Domain d = info.getDomain();
        assertEquals("greebe", d.getName());
        assertEquals("myauthrealm", d.getRealm());
        String[] aliases = d.getAliases();
        assertEquals(2, aliases.length);
        // NOTE: order is not important
        Arrays.sort(aliases);
        assertEquals("horned", aliases[0]);
        assertEquals("pie-billed", aliases[1]);

        domainManagerControl.verify();
        coreContextControl.verify();
    }
}
