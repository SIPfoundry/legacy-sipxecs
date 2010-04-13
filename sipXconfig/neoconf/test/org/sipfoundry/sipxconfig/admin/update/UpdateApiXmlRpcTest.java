/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.update;

import java.util.Arrays;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SoftwareAdminApi;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class UpdateApiXmlRpcTest extends TestCase {

    public void testGetCurrentVersionMaster() {

        Location location = new Location();
        location.setFqdn("sipx.example.com");

        LocationsManager lm = createMock(LocationsManager.class);
        lm.getPrimaryLocation();
        expectLastCall().andReturn(location);

        SoftwareAdminApi api = createMock(SoftwareAdminApi.class);
        api.exec("sipx.example.com", "version");
        expectLastCall().andReturn(Arrays.asList("/path/to/file"));
        api.execStatus("sipx.example.com", "version");
        expectLastCall().andReturn("DONE");

        ApiProvider<SoftwareAdminApi> apiProvider = createMock(ApiProvider.class);
        apiProvider.getApi("https://sipx.example.com:8092/RPC2");
        expectLastCall().andReturn(api);

        replay(lm, apiProvider, api);

        // test output on master system
        DummyUpdateApi out = new DummyUpdateApi();
        out.setFileLines(Arrays.asList("version:sipxcommons 3.11.15"));
        out.setLocationsManager(lm);
        out.setSoftwareAdminApiProvider(apiProvider);

        assertEquals("3.11.15", out.getCurrentVersion());

        verify(lm, apiProvider, api);
    }

    public void testGetCurrentVersionDistributed() {

        Location location = new Location();
        location.setFqdn("sipx.example.com");

        LocationsManager lm = createMock(LocationsManager.class);
        lm.getPrimaryLocation();
        expectLastCall().andReturn(location);

        SoftwareAdminApi api = createMock(SoftwareAdminApi.class);
        api.exec("sipx.example.com", "version");
        expectLastCall().andReturn(Arrays.asList("/path/to/file"));
        api.execStatus("sipx.example.com", "version");
        expectLastCall().andReturn("DONE");

        ApiProvider<SoftwareAdminApi> apiProvider = createMock(ApiProvider.class);
        apiProvider.getApi("https://sipx.example.com:8092/RPC2");
        expectLastCall().andReturn(api);

        replay(lm, apiProvider, api);

        DummyUpdateApi out = new DummyUpdateApi();
        out.setFileLines(Arrays.asList("version:sipxcommserverlib 4.1.0-016788"));
        out.setLocationsManager(lm);
        out.setSoftwareAdminApiProvider(apiProvider);

        assertEquals("4.1.0-016788", out.getCurrentVersion());

        verify(lm, apiProvider, api);
    }

    public void testGetCurrentVersionSources() {

        Location location = new Location();
        location.setFqdn("sipx.example.com");

        LocationsManager lm = createMock(LocationsManager.class);
        lm.getPrimaryLocation();
        expectLastCall().andReturn(location);

        SoftwareAdminApi api = createMock(SoftwareAdminApi.class);
        api.exec("sipx.example.com", "version");
        expectLastCall().andReturn(Arrays.asList("/path/to/file"));
        api.execStatus("sipx.example.com", "version");
        expectLastCall().andReturn("DONE");

        ApiProvider<SoftwareAdminApi> apiProvider = createMock(ApiProvider.class);
        apiProvider.getApi("https://sipx.example.com:8092/RPC2");
        expectLastCall().andReturn(api);

        replay(lm, apiProvider, api);

        DummyUpdateApi out = new DummyUpdateApi();
        out.setFileLines(Arrays.asList("Could not determine version"));
        out.setLocationsManager(lm);
        out.setSoftwareAdminApiProvider(apiProvider);

        assertEquals("Could not determine version", out.getCurrentVersion());

        verify(lm, apiProvider, api);
    }

    public void testGetAvailableUpdates() {
        Location location = new Location();
        location.setFqdn("sipx.example.com");

        LocationsManager lm = createMock(LocationsManager.class);
        lm.getPrimaryLocation();
        expectLastCall().andReturn(location);

        SoftwareAdminApi api = createMock(SoftwareAdminApi.class);
        api.exec("sipx.example.com", "check-update");
        expectLastCall().andReturn(Arrays.asList("/path/to/file"));
        api.execStatus("sipx.example.com", "check-update");
        expectLastCall().andReturn("DONE");

        ApiProvider<SoftwareAdminApi> apiProvider = createMock(ApiProvider.class);
        apiProvider.getApi("https://sipx.example.com:8092/RPC2");
        expectLastCall().andReturn(api);

        replay(lm, apiProvider, api);

        DummyUpdateApi out = new DummyUpdateApi();
        out.setFileLines(Arrays.asList("# Package|Current|Next", "sipXecs|3.11.5|4.0", "sipXconfig|3.11.5|4.0"));
        out.setLocationsManager(lm);
        out.setSoftwareAdminApiProvider(apiProvider);

        List<PackageUpdate> availableUpdates = out.getAvailableUpdates();
        assertEquals(2, availableUpdates.size());
        PackageUpdate p1 = availableUpdates.get(0);
        assertEquals("sipXecs", p1.getPackageName());
        assertEquals("3.11.5", p1.getCurrentVersion());
        assertEquals("4.0", p1.getUpdatedVersion());

        PackageUpdate p2 = availableUpdates.get(0);
        assertEquals("sipXecs", p2.getPackageName());
        assertEquals("3.11.5", p2.getCurrentVersion());
        assertEquals("4.0", p2.getUpdatedVersion());

        verify(lm, apiProvider, api);
    }

    public void testInstallUpdates() {
        Location l1 = new Location();
        l1.setFqdn("sipx.example.com");
        l1.setUniqueId();
        l1.setPrimary(true);

        Location l2 = new Location();
        l2.setFqdn("sipx2.example.com");
        l2.setUniqueId();
        l2.setPrimary(false);

        LocationsManager lm = createMock(LocationsManager.class);
        lm.getPrimaryLocation();
        expectLastCall().andReturn(l1);
        lm.getLocations();
        expectLastCall().andReturn(new Location[] {
            l1, l2
        });

        SoftwareAdminApi api = createMock(SoftwareAdminApi.class);
        api.exec("sipx.example.com", "update");
        expectLastCall().andReturn(Arrays.asList("/path/to/file"));
        api.exec("sipx.example.com", "update");
        expectLastCall().andReturn(Arrays.asList("/path/to/file"));

        ApiProvider<SoftwareAdminApi> apiProvider = createMock(ApiProvider.class);
        apiProvider.getApi("https://sipx.example.com:8092/RPC2");
        expectLastCall().andReturn(api);
        apiProvider.getApi("https://sipx2.example.com:8092/RPC2");
        expectLastCall().andReturn(api);

        replay(lm, apiProvider, api);

        UpdateApiXmlRpc out = new UpdateApiXmlRpc();
        out.setLocationsManager(lm);
        out.setSoftwareAdminApiProvider(apiProvider);

        out.installUpdates();

        verify(lm, apiProvider, api);
    }

    static class DummyUpdateApi extends UpdateApiXmlRpc {
        private List<String> m_lines;

        void setFileLines(List<String> lines) {
            m_lines = lines;
        }

        @Override
        protected List<String> retrieveRemoteFile(String fileUrl) {
            return m_lines;
        }
    }

}
