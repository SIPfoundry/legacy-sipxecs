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

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import static org.easymock.EasyMock.createStrictMock;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class ReplicationTriggerTestIntegration extends IntegrationTestCase {

    private ReplicationTrigger m_trigger;
    private SipxReplicationContext m_originalSipxReplicationContext;
    private SettingDao m_dao;

    public void setReplicationTrigger(ReplicationTrigger trigger) {
        m_trigger = trigger;
    }

    public void setSettingDao(SettingDao dao) {
        m_dao = dao;
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_originalSipxReplicationContext = sipxReplicationContext;
    }

    @Override
    protected void onTearDownAfterTransaction() throws Exception {
        // restore trigger state...
        m_trigger.setReplicationContext(m_originalSipxReplicationContext);
    }

    /**
     * Test that saving a user group in db actually triggers a replication
     */
    public void testNewUserGroup() throws Exception {
        SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
        replicationContext.generate(DataSet.PERMISSION);
        replicationContext.generate(DataSet.USER_LOCATION);
        replay(replicationContext);
        m_trigger.setReplicationContext(replicationContext);

        Group g = new Group();
        g.setName("replicationTriggerTest");
        g.setResource(User.GROUP_RESOURCE_ID);
        m_dao.saveGroup(g);

        verify(replicationContext);
    }

    public void testUpdateUserGroup() throws Exception {
        loadDataSet("admin/commserver/imdb/UserGroupSeed.db.xml");

        SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
        replicationContext.generate(DataSet.PERMISSION);
        replicationContext.generate(DataSet.USER_LOCATION);
        replay(replicationContext);

        m_trigger.setReplicationContext(replicationContext);
        Group g = m_dao.getGroup(new Integer(1000));
        m_dao.saveGroup(g);

        verify(replicationContext);
    }

    /**
     * Test that replication happens at app startup if the replicateOnStartup property is set
     */
    public void testReplicateOnStartup() throws Exception {
        SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
        replicationContext.generateAll();
        replay(replicationContext);
        m_trigger.setReplicationContext(replicationContext);

        m_trigger.setReplicateOnStartup(true);
        m_trigger.onApplicationEvent(new ApplicationInitializedEvent(new Object()));

        verify(replicationContext);
    }

    /**
     * Test that replication doesn't happen at app startup if the replicateOnStartup property is
     * off
     */
    public void testNoReplicateOnStartup() throws Exception {
        SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
        replay(replicationContext);
        m_trigger.setReplicationContext(replicationContext);

        m_trigger.setReplicateOnStartup(false);
        m_trigger.onApplicationEvent(new ApplicationInitializedEvent(new Object()));

        verify(replicationContext);
    }
}
