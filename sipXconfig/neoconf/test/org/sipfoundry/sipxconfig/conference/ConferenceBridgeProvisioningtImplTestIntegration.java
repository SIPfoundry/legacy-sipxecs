/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

// TODO: improve weak test: it should check the content of the generated file
public class ConferenceBridgeProvisioningtImplTestIntegration extends IntegrationTestCase {

    private ConferenceBridgeContext m_context;

    private DeviceDefaults m_phoneDefaults;

    private ModelFilesContext m_modelFilesContext;

    private DomainManager m_domainManager;
    
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
    
    public void setConferenceBridgeContext(ConferenceBridgeContext context) {
        m_context = context;
    }

    public void setModelFilesContext(ModelFilesContext modelFilesContext) {
        m_modelFilesContext = modelFilesContext;
    }

    public void setPhoneDefaults(DeviceDefaults phoneDefaults) {
        m_phoneDefaults = phoneDefaults;
    }

    public void testGenerateConfigurationData() throws Exception {
        loadDataSetXml("admin/dialplan/sbc/domain.xml");
        loadDataSet("conference/users.db.xml");
        loadDataSet("conference/participants.db.xml");

        SipxReplicationContext replication = createMock(SipxReplicationContext.class);
        replication.replicate(isA(ConferenceConfiguration.class));
        replay(replication);

        ConferenceBridgeProvisioningImpl impl = new ConferenceBridgeProvisioningImpl() {
            @Override
            public ConferenceConfiguration createConferenceConfiguration() {
                ConferenceConfiguration config = new ConferenceConfiguration();
                config.setDomainManager(m_domainManager);
                return config;
            }
        };
        impl.setSipxReplicationContext(replication);

        Bridge bridge = m_context.loadBridge(new Integer(2005));
        bridge.setModelFilesContext(m_modelFilesContext);
        bridge.setSystemDefaults(m_phoneDefaults);
        List<Conference> conferences = new ArrayList<Conference>(bridge.getConferences());
        for (Conference c : conferences) {
            c.setModelFilesContext(m_modelFilesContext);
        }

        impl.generateConfigurationData(bridge, conferences);

        verify(replication);
    }
}
