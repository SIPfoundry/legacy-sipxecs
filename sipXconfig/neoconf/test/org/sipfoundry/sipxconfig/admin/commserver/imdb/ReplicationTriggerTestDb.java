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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.springframework.context.ApplicationContext;

public class ReplicationTriggerTestDb extends SipxDatabaseTestCase {

    private ReplicationTrigger m_trigger;
    private SipxReplicationContext m_oldReplicationContext;
    private IMocksControl m_replicationContextCtrl;
    private SipxReplicationContext m_replicationContext;
    private SettingDao m_dao;
    private IMocksControl m_parkOrbitsContextCtrl;
    private ParkOrbitContext m_parkOrbitsContext;
    private ParkOrbitContext m_oldParkOrbitContext;
    private IMocksControl m_speedDialManagerControl;
    private SpeedDialManager m_speedDialManager;
    private SpeedDialManager m_oldSpeedDialManager;

    @Override
    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_trigger = (ReplicationTrigger) app.getBean("replicationTrigger");
        m_oldParkOrbitContext = m_trigger.getParkOrbitContext();
        m_oldReplicationContext = m_trigger.getReplicationContext();
        m_oldSpeedDialManager = m_trigger.getSpeedDialManager();

        TestHelper.cleanInsert("ClearDb.xml");

        m_replicationContextCtrl = EasyMock.createStrictControl();
        m_replicationContext = m_replicationContextCtrl.createMock(SipxReplicationContext.class);

        m_parkOrbitsContextCtrl = EasyMock.createControl();
        m_parkOrbitsContext = m_parkOrbitsContextCtrl.createMock(ParkOrbitContext.class);

        m_speedDialManagerControl = EasyMock.createControl();
        m_speedDialManager = m_speedDialManagerControl.createMock(SpeedDialManager.class);

        m_dao = (SettingDao) app.getBean("settingDao");
    }

    @Override
    protected void tearDown() {
        m_trigger.setReplicationContext(m_oldReplicationContext);
        m_trigger.setParkOrbitContext(m_oldParkOrbitContext);
        m_trigger.setSpeedDialManager(m_oldSpeedDialManager);

        m_parkOrbitsContextCtrl.verify();
        m_replicationContextCtrl.verify();
        m_speedDialManagerControl.verify();
    }

    /**
     * Call this method to set up the mock control to expect exactly one call to
     * m_replicationContext.generate .
     */
    private void expectOneCallToGenerate() {
        m_replicationContext.generate(DataSet.PERMISSION);
        m_replicationContext.generate(DataSet.USER_LOCATION);
        m_replicationContextCtrl.replay();
        m_parkOrbitsContextCtrl.replay();
        m_speedDialManagerControl.replay();
    }

    /**
     * Call this method to set up the mock control to expect exactly one call to
     * m_replicationContext.generateAll .
     */
    private void expectOneCallToGenerateAll() {
        m_replicationContext.generateAll();
        m_replicationContextCtrl.replay();
        m_parkOrbitsContext.activateParkOrbits();
        m_parkOrbitsContextCtrl.replay();
        m_speedDialManager.activateResourceList();
        m_speedDialManagerControl.replay();
    }

    /**
     * Call this method to set up the mock control to expect no calls.
     */
    private void expectNoCalls() {
        m_replicationContextCtrl.replay();
        m_parkOrbitsContextCtrl.replay();
        m_speedDialManagerControl.replay();
    }

    /**
     * Test that saving a user group in db actually triggers a replication
     */
    public void testNewUserGroup() throws Exception {
        m_trigger.setReplicationContext(m_replicationContext);
        Group g = new Group();
        g.setName("replicationTriggerTest");
        g.setResource(User.GROUP_RESOURCE_ID);
        expectOneCallToGenerate();
        m_dao.saveGroup(g);
    }

    public void testUpdateUserGroup() throws Exception {
        TestHelper.cleanInsertFlat("admin/commserver/imdb/UserGroupSeed.db.xml");
        m_trigger.setReplicationContext(m_replicationContext);
        Group g = m_dao.getGroup(new Integer(1000));
        expectOneCallToGenerate();
        m_dao.saveGroup(g);
    }

    /**
     * Test that replication happens at app startup if the replicateOnStartup property is set
     */
    public void testReplicateOnStartup() throws Exception {
        m_trigger.setReplicationContext(m_replicationContext);
        m_trigger.setParkOrbitContext(m_parkOrbitsContext);
        m_trigger.setSpeedDialManager(m_speedDialManager);
        m_trigger.setReplicateOnStartup(true);
        expectOneCallToGenerateAll();
        m_trigger.onApplicationEvent(new ApplicationInitializedEvent(new Object()));
    }

    /**
     * Test that replication doesn't happen at app startup if the replicateOnStartup property is
     * off
     */
    public void testNoReplicateOnStartup() throws Exception {
        m_trigger.setReplicationContext(m_replicationContext);
        m_trigger.setReplicateOnStartup(false);
        expectNoCalls();
        m_trigger.onApplicationEvent(new ApplicationInitializedEvent(new Object()));
    }
}
