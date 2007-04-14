package org.sipfoundry.sipxconfig.api;

import java.util.Arrays;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SystemServiceTest extends TestCase {
    
    public void testSystemInfo() throws Exception {
        org.sipfoundry.sipxconfig.domain.Domain testDomain = new org.sipfoundry.sipxconfig.domain.Domain("greebe");
        testDomain.addAlias("horned");
        testDomain.addAlias("pie-billed");
        IMocksControl domainManagerControl = EasyMock.createStrictControl();
        DomainManager domainManager = domainManagerControl.createMock(DomainManager.class);
        domainManager.getDomain();
        domainManagerControl.andReturn(testDomain);
        domainManagerControl.replay();

        SystemServiceImpl service = new SystemServiceImpl();
        service.setDomainManager(domainManager);
        SystemInfo info = service.systemInfo();
        assertNotNull(info);
        Domain d = info.getDomain();
        assertEquals("greebe", d.getName());
        String[] aliases = d.getAliases(); 
        assertEquals(2, aliases.length);
        // NOTE: order is not important
        Arrays.sort(aliases);
        assertEquals("horned", aliases[0]);
        assertEquals("pie-billed", aliases[1]);
        
        domainManagerControl.verify();
    }
}
