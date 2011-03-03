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
import java.util.Collections;

import org.apache.commons.codec.binary.Base64;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContextImpl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.device.InMemoryConfiguration;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.springframework.beans.factory.BeanFactory;

public class ReplicationManagerImplTest extends MongoTestCase {

    private static final Location[] LOCATIONS = new Location[] {
        new Location(), new Location()
    };
    public final static String DOMAIN = "mydomain.org";
    private LocationsManager m_locationsManager;
    private ReplicationManagerImpl m_out;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        m_locationsManager = TestUtil.getMockLocationsManager();

        m_out = new ReplicationManagerImpl();
        m_out.setLocationsManager(m_locationsManager);
        m_out.setAuditLogContext(new AuditLogContextImpl());
    }

    public void testReplicateFile() throws Exception {
        final FileApi fileApi = createMock(FileApi.class);

        String content = "1234";
        fileApi.replace("sipx.example.org", "/etc/sipxecs/domain-config", 0644, encode(content));
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

    /*public void testReplicateData() {
        Aliases dsg = new Aliases();
        ReplicableProvider prov = createMock(ReplicableProvider.class);
        prov.getAliasMappings();
        expectLastCall().andReturn(Collections.EMPTY_MAP).anyTimes();
        dsg.setAliasProvider(prov);

        BeanFactory factory = createMock(BeanFactory.class);
        for (DataSet dataSet : DataSet.getEnumList()) {
            String beanName = dataSet.getBeanName();
            factory.getBean(beanName, DataSetGenerator.class);
            expectLastCall().andReturn(dsg).anyTimes();
        }
        replay(prov, factory);
        m_out.setBeanFactory(factory);
        m_out.replicateAllData();

        verify(factory);
    }

    public void testReplicateEntity() {
        Replicable entity = createMock(Replicable.class);
        entity.getDataSets();
        expectLastCall().andReturn(Collections.singleton(DataSet.ALIAS)).atLeastOnce();
        entity.getName();// it actually will get into the error block
        expectLastCall().andReturn("blahblah").atLeastOnce();
        entity.getAliasMappings("domain.org");
        expectLastCall().andReturn(Collections.EMPTY_MAP);
        entity.getIdentity("domain.org");
        expectLastCall().andReturn("User1").anyTimes();

        Aliases dsg = new Aliases();
        AliasProvider prov = createMock(AliasProvider.class);
        prov.getAliasMappings();
        expectLastCall().andReturn(Collections.EMPTY_MAP).anyTimes();
        dsg.setAliasProvider(prov);

        CoreContext cc = createMock(CoreContext.class);
        cc.getDomainName();
        expectLastCall().andReturn("domain.org").anyTimes();
        dsg.setCoreContext(cc);

        BeanFactory factory = createMock(BeanFactory.class);
        String beanName = DataSet.ALIAS.getBeanName();
        factory.getBean(beanName, DataSetGenerator.class);
        expectLastCall().andReturn(dsg).anyTimes();
        replay(factory, entity, prov, cc);
        m_out.setBeanFactory(factory);
        m_out.replicateEntity(entity);

        verify(factory, entity);
    }*/

    private String encode(String content) throws UnsupportedEncodingException {
        byte[] encoded = Base64.encodeBase64(content.getBytes("US-ASCII"));
        return new String(encoded, "US-ASCII");
    }
}
