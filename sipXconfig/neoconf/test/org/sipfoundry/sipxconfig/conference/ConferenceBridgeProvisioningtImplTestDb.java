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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class ConferenceBridgeProvisioningtImplTestDb extends SipxDatabaseTestCase {

    private ConferenceBridgeContext m_context;

    protected void setUp() throws Exception {
        m_context = (ConferenceBridgeContext) TestHelper.getApplicationContext().getBean(
                ConferenceBridgeContext.CONTEXT_BEAN_NAME);

        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("conference/users.db.xml");
    }
    
    public void testGenerateAdmissionData() throws Exception {
        TestHelper.insertFlat("conference/participants.db.xml");
        Bridge bridge = m_context.loadBridge(new Integer(2005));

        IMocksControl hibernateCtrl = org.easymock.classextension.EasyMock.createControl();
        HibernateTemplate hibernate = hibernateCtrl.createMock(HibernateTemplate.class);
        hibernate.loadAll(Conference.class);
        hibernateCtrl.andReturn(new ArrayList(bridge.getConferences()));
        hibernateCtrl.replay();

        IMocksControl replicationCtrl = EasyMock.createControl();
        SipxReplicationContext replication = replicationCtrl.createMock(SipxReplicationContext.class);
        EasyMock.anyObject();
        replication.replicate(null);
        replicationCtrl.replay();

        ConferenceBridgeProvisioningImpl impl = new ConferenceBridgeProvisioningImpl();
        impl.setHibernateTemplate(hibernate);
        impl.setSipxReplicationContext(replication);
        
        List conferences = hibernate.loadAll(Conference.class);

        impl.generateAdmissionData(bridge, conferences);

        replicationCtrl.verify();
        hibernateCtrl.verify();
    }
    
    public void testGenerateConfigurationData() throws Exception {
        TestHelper.insertFlat("conference/participants.db.xml");
        Bridge bridge = m_context.loadBridge(new Integer(2005));

        IMocksControl hibernateCtrl = org.easymock.classextension.EasyMock.createControl();
        HibernateTemplate hibernate = hibernateCtrl.createMock(HibernateTemplate.class);
        hibernate.loadAll(Conference.class);
        hibernateCtrl.andReturn(new ArrayList(bridge.getConferences()));
        hibernateCtrl.replay();

        IMocksControl replicationCtrl = EasyMock.createControl();
        SipxReplicationContext replication = replicationCtrl.createMock(SipxReplicationContext.class);
        EasyMock.anyObject();
        replication.replicate(null);
        replicationCtrl.replay();

        ConferenceBridgeProvisioningImpl impl = new ConferenceBridgeProvisioningImpl();
        impl.setHibernateTemplate(hibernate);
        impl.setSipxReplicationContext(replication);
        
        List conferences = hibernate.loadAll(Conference.class);

        impl.generateConfigurationData(bridge, conferences);

        replicationCtrl.verify();
        hibernateCtrl.verify();
    }   
}
