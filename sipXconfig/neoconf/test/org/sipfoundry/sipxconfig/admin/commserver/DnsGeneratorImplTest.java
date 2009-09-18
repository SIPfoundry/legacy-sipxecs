/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import static org.easymock.EasyMock.*;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

import junit.framework.TestCase;

import static org.sipfoundry.sipxconfig.test.TestUtil.getMockSipxServiceManager;

public class DnsGeneratorImplTest extends TestCase {
    public void testGenerate() {
        // Set up the per-location services
        Collection<SipxService> sipxServices = new ArrayList<SipxService>();
        SipxService proxyService = new SipxProxyService();
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        sipxServices.add(proxyService);
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        sipxServices.add(registrarService);

        // This provides the proxy service bean name
        SipxServiceManager sm = getMockSipxServiceManager(true, proxyService);

        Location l1 = new Location();
        l1.setUniqueId();
        l1.setAddress("10.1.1.1");
        l1.setFqdn("primary.ex.org");
        l1.setPrimary(true);
        l1.setServiceDefinitions(sipxServices);

        Location l2 = new Location();
        l2.setUniqueId();
        l2.setAddress("10.1.1.2");
        l2.setFqdn("s2.ex.org");

        Location l3 = new Location();
        l3.setUniqueId();
        l3.setAddress("10.1.1.3");
        l3.setFqdn("s3.ex.org");

        Location l4 = new Location();
        l4.setUniqueId();
        l4.setAddress("10.1.1.4");
        l4.setFqdn("redund.ex.org");
        l4.setServiceDefinitions(sipxServices);

        // DNS generator calls getLocations twice each time it's called.
        LocationsManager lm = createMock(LocationsManager.class);
        lm.getPrimaryLocation();
        expectLastCall().andReturn(l1).anyTimes();
        lm.getLocations();
        expectLastCall().andReturn(array(l1)).times(2);
        lm.getLocations();
        expectLastCall().andReturn(array(l1, l2, l3)).times(4);
        lm.getLocations();
        expectLastCall().andReturn(array(l1, l3, l4)).times(2);

        final ZoneAdminApi api = createStrictMock(ZoneAdminApi.class);

        // primary only
        api.generateDns("primary.ex.org", "primary.ex.org/10.1.1.1 --zone --serial 1 --provide-dns");

        // all 3 locations
        api.generateDns("primary.ex.org",
           "primary.ex.org/10.1.1.1 -o s2.ex.org/10.1.1.2 -o s3.ex.org/10.1.1.3 --zone --serial 2 --provide-dns");

        // deleting 2nd...
        api.generateDns("primary.ex.org",
            "primary.ex.org/10.1.1.1 -o s3.ex.org/10.1.1.3 --zone --serial 3 --provide-dns");

        // primary and redundant locations
        api.generateDns("primary.ex.org",
            "primary.ex.org/10.1.1.1 redund.ex.org/10.1.1.4 -o s3.ex.org/10.1.1.3 --zone --serial 4 --provide-dns");

        replay(lm, api);

        ApiProvider<ZoneAdminApi> apiProvider = new ApiProvider<ZoneAdminApi>() {
            public ZoneAdminApi getApi(String serviceUrl) {
                return api;
            }
        };

        DnsGeneratorImpl impl = new DnsGeneratorImpl();
        impl.setLocationsManager(lm);
        impl.setSipxServiceManager(sm);
        impl.setZoneAdminApiProvider(apiProvider);

        // primary only
        impl.generate();

        // all 3 locations
        impl.generate();

        // deleting 2nd...
        impl.generateWithout(l2);

        // primary and redundant locations
        impl.generate();

        verify(lm, api);
    }

    private static <T> T[] array(T... items) {
        return items;
    }
}
