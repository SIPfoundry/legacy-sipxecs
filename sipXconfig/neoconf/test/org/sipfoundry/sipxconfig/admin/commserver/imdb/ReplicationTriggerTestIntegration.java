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
import java.util.Collections;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ContactInformationDaoListener;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeer;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeerManager;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
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
    private TlsPeerManager m_tlsPeerManager;
    private PermissionManager m_permissionManager;
    private ForwardingContext m_forwardingContext;
    private ReplicationManagerImpl m_replicationManager;
    private static String DBNAME = "imdb";
    private static String COLL_NAME = "entity";

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        MongoTestCaseHelper.initMongo(DBNAME, COLL_NAME);
    }

    @Override
    protected void onTearDownInTransaction() throws Exception {
        super.onTearDownInTransaction();
        MongoTestCaseHelper.dropDb(DBNAME);
    }

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

    public void testUpdateUserGroup() throws Exception {
        loadDataSet("admin/commserver/imdb/UserGroupSeed2.db.xml");
        loadDataSetXml("domain/DomainSeed.xml");

        Group g = m_dao.getGroup(new Integer(1000));
        User user = m_coreContext.loadUser(1001);
        user.setPermissionManager(m_permissionManager);
        User user2 = m_coreContext.loadUser(1002);
        user2.setPermissionManager(m_permissionManager);

        SortedSet<Group> groups = new TreeSet<Group>();
        groups.add(g);
        user.setGroups(groups);

        m_coreContext.saveUser(user);
        MongoTestCaseHelper.assertObjectWithIdPresent("User1001");
        MongoTestCaseHelper.assertObjectWithIdNotPresent("User1002");
        m_dao.saveGroup(g);

        MongoTestCaseHelper.assertObjectWithIdPresent("User1002");
    }

    public void testReplicateOnTlsPeerCreation() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        TlsPeer peer = m_tlsPeerManager.newTlsPeer();
        peer.setName("test");

        m_tlsPeerManager.saveTlsPeer(peer);
        MongoTestCaseHelper.assertObjectWithIdPresent("TlsPeer1");

    }

    /**
     * Test that replication happens at app startup if the replicateOnStartup property is set
     */
    public void testReplicateOnStartup() throws Exception {
        ReplicationManager replicationContext = createStrictMock(ReplicationManager.class);
        replicationContext.replicateAllData();
        replay(replicationContext);
        m_trigger.setReplicationManager(replicationContext);

        m_trigger.setReplicateOnStartup(true);
        m_trigger.onApplicationEvent(new ApplicationInitializedEvent(new Object()));

        verify(replicationContext);
        m_trigger.setReplicationManager(m_replicationManager);
    }

    /**
     * Test that replication doesn't happen at app startup if the replicateOnStartup property is
     * off
     */
    public void testNoReplicateOnStartup() throws Exception {
        ReplicationManager replicationContext = createStrictMock(ReplicationManager.class);
        replay(replicationContext);
        m_trigger.setReplicationManager(replicationContext);

        m_trigger.setReplicateOnStartup(false);
        m_trigger.onApplicationEvent(new ApplicationInitializedEvent(new Object()));

        verify(replicationContext);
        m_trigger.setReplicationManager(m_replicationManager);
    }

    /**
     * Tests that replication is triggered when branch with users deleted
     */
    /*
     * public void testDeleteBranchWithUser() throws Exception {
     * loadDataSet("branch/attached_branches.db.xml");
     * 
     * SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
     * replicationContext.generate(DataSet.USER_LOCATION);
     * replicationContext.replicate(m_contactInformationConfig); replay(replicationContext);
     * m_contactInformationDaoListener.setSipxReplicationContext(replicationContext);
     * m_trigger.setReplicationContext(replicationContext);
     * 
     * Collection<Integer> allSelected = new ArrayList<Integer>(); allSelected.add(1000);
     * m_branchManager.deleteBranches(allSelected);
     * 
     * verify(replicationContext); }
     */

    /**
     * Tests that replication is triggered when branch with users is saved
     */
    /*
     * public void testSaveBranchWithUser() throws Exception {
     * loadDataSet("branch/attached_branches.db.xml");
     * 
     * SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
     * replicationContext.generate(DataSet.USER_LOCATION);
     * replicationContext.replicate(m_contactInformationConfig); replay(replicationContext);
     * m_contactInformationDaoListener.setSipxReplicationContext(replicationContext);
     * m_trigger.setReplicationContext(replicationContext);
     * 
     * Branch branch = m_branchManager.getBranch(1000);
     * 
     * m_branchManager.saveBranch(branch);
     * 
     * verify(replicationContext); }
     */

    /**
     * Tests that no replication when branch without users is deleted
     */
    /*
     * public void testDeleteBranchesWithoutUser() throws Exception {
     * loadDataSet("branch/branches.db.xml");
     * 
     * SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
     * replay(replicationContext); m_trigger.setReplicationContext(replicationContext);
     * 
     * m_branchManager.deleteBranches(Arrays.asList(1, 2, 3, 4, 5));
     * 
     * verify(replicationContext); }
     */

    /**
     * Tests that no replication is done when branch without users is saved
     */
    /*
     * public void testSaveBranchesWithoutUser() throws Exception {
     * 
     * SipxReplicationContext replicationContext = createStrictMock(SipxReplicationContext.class);
     * replay(replicationContext); m_trigger.setReplicationContext(replicationContext);
     * 
     * //Save new branch Branch newBranch = new Branch(); newBranch.setName("testBranch");
     * m_branchManager.saveBranch(newBranch); flush();
     * 
     * //Save an existing branch Branch existingBranch = m_branchManager.getBranch("testBranch");
     * m_branchManager.saveBranch(existingBranch); flush();
     * 
     * verify(replicationContext); }
     */

    public void setContactInformationConfig(ConfigurationFile contactInformationConfig) {
        m_contactInformationConfig = contactInformationConfig;
    }

    public void setContactInformationDaoListener(ContactInformationDaoListener contactInformationDaoListener) {
        m_contactInformationDaoListener = contactInformationDaoListener;
    }

    public void setTlsPeerManager(TlsPeerManager peerManager) {
        m_tlsPeerManager = peerManager;
    }

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replicationManager) {
        m_replicationManager = replicationManager;
    }

}
