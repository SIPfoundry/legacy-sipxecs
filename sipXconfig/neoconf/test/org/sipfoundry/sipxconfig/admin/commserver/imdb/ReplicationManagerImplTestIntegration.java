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
import static org.easymock.EasyMock.reset;
import static org.easymock.EasyMock.verify;

import java.io.UnsupportedEncodingException;

import org.apache.commons.codec.binary.Base64;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.device.InMemoryConfiguration;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

public class ReplicationManagerImplTestIntegration extends ImdbTestCase {
    private AuditLogContext m_auditLogContext;
    private ReplicationManagerImpl m_out;
    private FileApi m_fileApi;

    @Override
    public void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_out = new ReplicationManagerImpl();
        m_out.setAuditLogContext(m_auditLogContext);
        m_fileApi = createMock(FileApi.class);

        ApiProvider<FileApi> provider = new ApiProvider<FileApi>() {
            public FileApi getApi(String serviceUrl) {
                return m_fileApi;
            }
        };

        m_out.setFileApiProvider(provider);
    }

    public void testNoReplicateToInappropriateLocations() throws Exception {
        Location l = new Location();
        ConfigurationFile file = new InMemoryConfiguration(TestHelper.getTestDirectory(), "domain-config", "1234");

        replay(m_fileApi);
        l.setRegistered(true);
        l.setReplicateConfig(false);
        m_out.replicateFile("primary", l, file);
        verify(m_fileApi);
        
        reset(m_fileApi);
        replay(m_fileApi);
        l.setRegistered(false);
        l.setReplicateConfig(true);
        m_out.replicateFile("primary", l, file);
        verify(m_fileApi);        
    }
    
    public void testReplicateFile() throws Exception {
        String content = "1234";        
        ConfigurationFile file = new InMemoryConfiguration(TestHelper.getTestDirectory(), "domain-config", content);        
        Location l = new Location();
        m_fileApi.replace("primary", TestHelper.getTestDirectory() + "/domain-config", 0644, encode(content));
        expectLastCall().andReturn(true).once();
        replay(m_fileApi);
        l.setRegistered(true);
        l.setReplicateConfig(true);
        m_out.replicateFile("primary", l, file);
        verify(m_fileApi);
    }

    private String encode(String content) throws UnsupportedEncodingException {
        byte[] encoded = Base64.encodeBase64(content.getBytes("US-ASCII"));
        return new String(encoded, "US-ASCII");
    }

    public void setAuditLogContext(AuditLogContext auditLogContext) {
        m_auditLogContext = auditLogContext;
    }
}
