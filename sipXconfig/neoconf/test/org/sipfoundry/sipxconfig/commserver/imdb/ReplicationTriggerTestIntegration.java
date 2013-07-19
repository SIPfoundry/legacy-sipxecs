/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectPresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValueNotPresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValuePresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdNotPresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdPresent;

import java.util.Collections;
import java.util.SortedSet;
import java.util.TreeSet;
import java.util.concurrent.ExecutorService;

import org.easymock.EasyMock;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.imdb.ReplicationTrigger.BranchDeleteWorker;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.test.ImdbTestCase;
import org.sipfoundry.sipxconfig.tls.TlsPeer;
import org.sipfoundry.sipxconfig.tls.TlsPeerManager;

import com.mongodb.BasicDBObject;

public class ReplicationTriggerTestIntegration extends ImdbTestCase {
    private ReplicationTrigger m_trigger;
    private SettingDao m_dao;
    private BranchManager m_branchManager;
    private TlsPeerManager m_tlsPeerManager;
    private ReplicationManagerImpl m_replicationManager;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        sql("domain/DomainSeed.sql");
    }

    // disabled this test due to the fact that CoreContext.getGroupMembersCount()
    // uses plain sql and for some reason the db is empty
    public void _testUpdateUserGroup() throws Exception {
        loadDataSet("commserver/imdb/UserGroupSeed2.db.xml");

        Group g = m_dao.getGroup(new Integer(1000));
        User user = getCoreContext().loadUser(1001);
        user.setPermissionManager(getPermissionManager());
        User user2 = getCoreContext().loadUser(1002);
        user2.setPermissionManager(getPermissionManager());

        SortedSet<Group> groups = new TreeSet<Group>();
        groups.add(g);
        user.setGroups(groups);

        getCoreContext().saveUser(user);
        assertObjectWithIdPresent(getEntityCollection(), "User1001");
        assertObjectWithIdNotPresent(getEntityCollection(), "User1002");
        m_dao.saveGroup(g);
        Thread.sleep(5000);
        assertObjectWithIdPresent(getEntityCollection(), "User1002");
    }

    public void testReplicateOnTlsPeerCreation() throws Exception {
        TlsPeer peer = m_tlsPeerManager.newTlsPeer();
        peer.setName("test");

        m_tlsPeerManager.saveTlsPeer(peer);
        commit();
        assertObjectPresent(getEntityCollection(), new BasicDBObject().append("ident", "~~tp~test@example.org"));
    }

    /*
     * This test simulates the actions that will be triggered if a branch is deleted.
     * It just simulates, b/c the async invocation poses some problems for Junit
     */
    public void testSimulatedDeleteBranch() throws Exception {
        loadDataSet("branch/attached_branches.db.xml");
        Branch b = m_branchManager.getBranch(1000);
        User u = getCoreContext().loadUser(1000);
        m_replicationManager.replicateEntity(u, DataSet.USER_LOCATION);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1000", MongoConstants.USER_LOCATION, "branch1");

        //verify that the replication is triggered
        ExecutorService executorService = EasyMock.createMock(ExecutorService.class);
        executorService.submit(EasyMock.isA(BranchDeleteWorker.class));
        EasyMock.expectLastCall().andReturn(null);
        executorService.shutdown();
        replay(executorService);
        m_trigger.setExecutorService(executorService);
        m_branchManager.deleteBranches(Collections.singletonList(b.getId()));
        
        verify(executorService);
        //verify that the user-branch relationship no longer exists and correct
        //entry is inserted in mongo
        evict(u);
        User u1 = getCoreContext().loadUser(1000);
        m_replicationManager.replicateEntity(u1, DataSet.USER_LOCATION);
        assertObjectWithIdFieldValueNotPresent(getEntityCollection(), "User1000", MongoConstants.USER_LOCATION, "branch1");
    }

    public void setTlsPeerManager(TlsPeerManager peerManager) {
        m_tlsPeerManager = peerManager;
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replicationManager) {
        m_replicationManager = replicationManager;
    }
    public void setReplicationTrigger(ReplicationTrigger trigger) {
        m_trigger = trigger;
    }

    public void setSettingDao(SettingDao dao) {
        m_dao = dao;
    }

    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }
}
