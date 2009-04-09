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

import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

import junit.framework.TestCase;

public class DnsGeneratorImplTest extends TestCase {

    public void testGenerate() {
        Location l1 = new Location();
        l1.setUniqueId();
        l1.setAddress("10.1.1.1");
        l1.setFqdn("primary.ex.org");
        l1.setPrimary(true);

        Location l2 = new Location();
        l2.setUniqueId();
        l2.setAddress("10.1.1.2");
        l2.setFqdn("s2.ex.org");

        Location l3 = new Location();
        l3.setUniqueId();
        l3.setAddress("10.1.1.3");
        l3.setFqdn("s3.ex.org");

        LocationsManager lm = createMock(LocationsManager.class);
        lm.getPrimaryLocation();
        expectLastCall().andReturn(l1).anyTimes();
        lm.getLocations();
        expectLastCall().andReturn(array(l1)).once();
        lm.getLocations();
        expectLastCall().andReturn(array(l1, l2, l3)).times(2);

        final ZoneAdminApi api = createStrictMock(ZoneAdminApi.class);

        // primary only
        api.generateDns("primary.ex.org", "primary.ex.org/10.1.1.1 --zone --serial 1 --provide-dns");

        // all 3 locations
        api.generateDns("primary.ex.org",
           "primary.ex.org/10.1.1.1 -o s2.ex.org/10.1.1.2 -o s3.ex.org/10.1.1.3 --zone --serial 2 --provide-dns");

        // deleting 2nd...
        api.generateDns("primary.ex.org",
            "primary.ex.org/10.1.1.1 -o s3.ex.org/10.1.1.3 --zone --serial 3 --provide-dns");

        replay(lm, api);

        ApiProvider<ZoneAdminApi> apiProvider = new ApiProvider<ZoneAdminApi>() {
            public ZoneAdminApi getApi(String serviceUrl) {
                return api;
            }
        };

        DnsGeneratorImpl impl = new DnsGeneratorImpl();
        impl.setLocationsManager(lm);
        impl.setZoneAdminApiProvider(apiProvider);

        // primary only
        impl.generate();

        // all 3 locations
        impl.generate();

        // deleting 2nd...
        impl.generateWithout(l2);

        verify(lm, api);
    }

    private static <T> T[] array(T... items) {
        return items;
    }
}
