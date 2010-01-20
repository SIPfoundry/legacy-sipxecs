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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ContactInformationDaoListener;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
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
    private CoreContext m_coreContext;
    private BranchManager m_branchManager;
    private ConfigurationFile m_contactInformationConfig;
    private ContactInformationDaoListener m_contactInformationDaoListener;

    public void setReplicationTrigger(ReplicationTrigger trigger) {
        m_trigger = trigger;
    }

    public void setSettingDao(SettingDao dao) {
        m_dao = dao;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_originalSipxReplicationContext = sipxReplicationContext;
    }

    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }

    @Override
    protected void onTearDownAfterTransaction() throws Exception {
        // restore trigger state...
        m_trigger.setReplicationContext(m_originalSipxReplicationContext);
        m_contactInformationDaoListener.
            setSipxReplicationContext(m_originalSipxReplicationContext);

    }

    /**
     * Test that saving a user group in db actually triggers a replication
     */
    public void testNewUserGroup() throws Exception {
        SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
        replicationContext.generate(DataSet.PERMISSION);
        replicationContext.generate(DataSet.USER_LOCATION);
        replicationContext.generate(DataSet.CALLER_ALIAS);
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
        replicationContext.generate(DataSet.CALLER_ALIAS);
        replay(replicationContext);

        m_trigger.setReplicationContext(replicationContext);
        Group g = m_dao.getGroup(new Integer(1000));
        m_dao.saveGroup(g);

        verify(replicationContext);
    }

    public void testReplicateSipxSaaConfig() throws Exception {
        loadDataSet("common/UserGroupSeed.db.xml");

        SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
        replicationContext.generateAll();
        replay(replicationContext);
        m_trigger.setReplicationContext(replicationContext);

        User testUser = m_coreContext.loadUser(1001);
        testUser.setIsShared(true);
        m_coreContext.saveUser(testUser);

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

    /**
     * Tests that replication is triggered when branch with users deleted
     */
    public void testDeleteBranchWithUser() throws Exception {
        loadDataSet("branch/attached_branches.db.xml");

        SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
        replicationContext.generate(DataSet.USER_LOCATION);
        replicationContext.replicate(m_contactInformationConfig);
        replay(replicationContext);
        m_contactInformationDaoListener.setSipxReplicationContext(replicationContext);
        m_trigger.setReplicationContext(replicationContext);

        Collection<Integer> allSelected = new ArrayList<Integer>();
        allSelected.add(1000);
        m_branchManager.deleteBranches(allSelected);

        verify(replicationContext);
    }

    /**
     * Tests that replication is triggered when branch with users is saved
     */
    public void testSaveBranchWithUser() throws Exception {
        loadDataSet("branch/attached_branches.db.xml");

        SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
        replicationContext.generate(DataSet.USER_LOCATION);
        replicationContext.replicate(m_contactInformationConfig);
        replay(replicationContext);
        m_contactInformationDaoListener.setSipxReplicationContext(replicationContext);
        m_trigger.setReplicationContext(replicationContext);

        Branch branch = m_branchManager.getBranch(1000);

        m_branchManager.saveBranch(branch);

        verify(replicationContext);
    }

    /**
     * Tests that no replication when branch without users is deleted
     */
    public void testDeleteBranchesWithoutUser() throws Exception {
        loadDataSet("branch/branches.db.xml");

        SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
        replay(replicationContext);
        m_trigger.setReplicationContext(replicationContext);

        m_branchManager.deleteBranches(Arrays.asList(1, 2, 3, 4, 5));

        verify(replicationContext);
    }

    /**
     * Tests that no replication is done when branch without users is saved
     */
    public void testSaveBranchesWithoutUser() throws Exception {

        SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
        replay(replicationContext);
        m_trigger.setReplicationContext(replicationContext);

        //Save new branch
        Branch newBranch = new Branch();
        newBranch.setName("testBranch");
        m_branchManager.saveBranch(newBranch);
        flush();

        //Save an existing branch
        Branch existingBranch = m_branchManager.getBranch("testBranch");
        m_branchManager.saveBranch(existingBranch);
        flush();

        verify(replicationContext);
    }

    public void setContactInformationConfig(ConfigurationFile contactInformationConfig) {
        m_contactInformationConfig = contactInformationConfig;
    }

    public void setContactInformationDaoListener(ContactInformationDaoListener contactInformationDaoListener) {
        m_contactInformationDaoListener = contactInformationDaoListener;
    }

}
