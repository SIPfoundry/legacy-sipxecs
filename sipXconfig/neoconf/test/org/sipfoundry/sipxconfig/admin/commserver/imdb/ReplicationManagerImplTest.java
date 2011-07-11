/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.io.UnsupportedEncodingException;

import junit.framework.TestCase;

import org.apache.commons.codec.binary.Base64;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContextImpl;
import org.sipfoundry.sipxconfig.device.InMemoryConfiguration;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

public class ReplicationManagerImplTest extends TestCase {

    private static final Location[] LOCATIONS = new Location[] {
        new Location(), new Location()
    };

    private LocationsManager m_locationsManager;
    private ReplicationManagerImpl m_out;

    @Override
    public void setUp() {
        m_locationsManager = TestUtil.getMockLocationsManager();

        m_out = new ReplicationManagerImplMock();
        m_out.setLocationsManager(m_locationsManager);
        m_out.setAuditLogContext(new AuditLogContextImpl());
    }

    public void testReplicateFile() throws Exception {
        final FileApi fileApi = createMock(FileApi.class);

        String content = "1234";
        fileApi.replace("sipx.example.org", "/etc/sipxecs/domain-config", 0644, encode(content), "final", "001");
        expectLastCall().andReturn(true).times(LOCATIONS.length);

        replay(fileApi);

        ApiProvider<FileApi> provider = new ApiProvider<FileApi>() {
            public FileApi getApi(String serviceUrl) {
                return fileApi;
            }
        };

        m_out.setFileApiProvider(provider);

        ConfigurationFile file = new InMemoryConfiguration("/etc/sipxecs", "domain-config", content);

        for (int i = 0; i < LOCATIONS.length; i++) {
            LOCATIONS[i].setRegistered(true);
        }
        m_out.replicateFile(LOCATIONS, file);

        verify(fileApi);
    }

    private String encode(String content) throws UnsupportedEncodingException {
        byte[] encoded = Base64.encodeBase64(content.getBytes("US-ASCII"));
        return new String(encoded, "US-ASCII");
    }

    private static class ReplicationManagerImplMock extends ReplicationManagerImpl {
        @Override
        protected String getSessionId() {
            return "001";
        }

    }
}
