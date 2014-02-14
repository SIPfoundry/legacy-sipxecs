/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.branch;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValueNotPresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValuePresent;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.TimeZone;

import org.dbunit.dataset.ITable;
import org.easymock.EasyMock;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.sipfoundry.sipxconfig.test.ImdbTestCase;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.MongoTestIntegration;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.time.NtpManager;

public class BranchManagerImplTestIntegration extends ImdbTestCase {

    private static final int NUM_BRANCHES = 5;

    private BranchManagerImpl m_branchManager;
    private CoreContext m_coreContext;
    private LocationsManager m_locationManager;

    public void setBranchManagerImpl(BranchManagerImpl branchManager) {
        m_branchManager = branchManager;
    }

    public void setLocationsManager(LocationsManager locationManager) {
        m_locationManager = locationManager;
    }

    @Override
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void testGetBranch() throws Exception {
        loadDataSet("branch/branches.db.xml");

        // Test getBranch(String name)
        assertNotNull(m_branchManager.getBranch("branch1"));
        assertNull(m_branchManager.getBranch("This branch does not exist"));

        // Test getBranch(Integer branchId)
        assertNotNull(m_branchManager.getBranch(1));
    }

    public void testGetBranches() throws Exception {
        loadDataSet("branch/branches.db.xml");

        List branches = m_branchManager.getBranches();

        // Check that we have the expected number of branches
        assertEquals(NUM_BRANCHES, branches.size());
    }

    public void testDeleteBranch() throws Exception {
        loadDataSet("branch/branches.db.xml");

        Branch branch1 = m_branchManager.getBranch("branch1");
        assertNotNull(branch1);
        m_branchManager.deleteBranches(Collections.singletonList(branch1.getId()));
        assertNull(m_branchManager.getBranch("branch1"));
    }

    public void testDeleteAttachedBranch() throws Exception {
        loadDataSet("branch/attached_branches.db.xml");

        Branch branch1 = m_branchManager.getBranch("branch1");
        assertNotNull(branch1);

        assertSame(branch1, m_coreContext.loadUser(1000).getBranch());
        assertSame(branch1, m_locationManager.getLocation(1000).getBranch());

        m_branchManager.deleteBranches(Collections.singletonList(branch1.getId()));

        assertNull(m_branchManager.getBranch("branch1"));
    }

    public void testDeleteBranches() throws Exception {
        loadDataSet("branch/branches.db.xml");

        // Check that we have the expected number of branches
        ITable branchTable = getConnection().createDataSet().getTable("branch");
        assertEquals(NUM_BRANCHES, branchTable.getRowCount());

        // Delete two branches
        m_branchManager.deleteBranches(Arrays.asList(1, 2));

        flush();

        // We should have reduced the branch count by two
        branchTable = getConnection().createDataSet().getTable("branch");
        assertEquals(3, branchTable.getRowCount());
    }

    public void testCreateUpdateBranch() throws Exception {
        loadDataSet("branch/branches.db.xml");

        // save a new branch with an existing name
        try {
            Branch branch = new Branch();
            branch.setName("branch1");
            m_branchManager.saveBranch(branch);
            flush();

            fail();
        } catch (UserException ex) {

        }

        // update an existing branch
        Branch branch4 = m_branchManager.getBranch("branch4");
        branch4.setDescription("Updated Description");
        m_branchManager.saveBranch(branch4);
        flush();
        assertEquals("Updated Description", m_branchManager.getBranch("branch4").getDescription());
    }

    public void testLoadBranchesByPage() throws Exception {
        loadDataSet("branch/branches.db.xml");

        List<Branch> page1 = m_branchManager.loadBranchesByPage(0, 5, new String[] {
            "name"
        }, true);
        // Check that we have the expected number of branches
        assertEquals(NUM_BRANCHES, page1.size());

        assertEquals("branch1", page1.get(0).getName());
        assertEquals("branch2", page1.get(1).getName());
        assertEquals("branch4", page1.get(3).getName());

        List<Branch> page2 = m_branchManager.loadBranchesByPage(0, 5, new String[] {
            "description"
        }, true);
        // Check that we have the expected number of branches
        assertEquals(NUM_BRANCHES, page2.size());

        assertEquals("fifth_", page2.get(0).getDescription());
        assertEquals("first_", page2.get(1).getDescription());
        assertEquals("second", page2.get(3).getDescription());
    }

    public void testSetup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        getHibernateTemplate().flush();
        loadDataSet("branch/branches_with-tz.db.xml");
        getHibernateTemplate().flush();

        m_branchManager.getBranches();

        Branch branch3 = m_branchManager.getBranch("branch30");
        Branch branch4 = m_branchManager.getBranch("branch40");
        Branch branch5 = m_branchManager.getBranch("branch50");
        assertEquals("Africa/Accra", branch4.getTimeZone());
        assertNull(branch3.getTimeZone());
        assertEquals("Africa/Abidjan", branch5.getTimeZone());

        SetupManager setupManager = createMock(SetupManager.class);
        setupManager.isFalse("update_branch_tz");
        expectLastCall().andReturn(true).times(2);

        setupManager.setTrue("update_branch_tz");
        expectLastCall().once();

        NtpManager ntpManager = EasyMock.createMock(NtpManager.class);
        ntpManager.getSystemTimezone();
        expectLastCall().andReturn(TimeZone.getTimeZone("GMT+1").getID()).anyTimes();

        replay(setupManager, ntpManager);
        m_branchManager.setNtpManager(ntpManager);
        m_branchManager.setup(setupManager);

        branch3 = m_branchManager.getBranch("branch30");
        assertTrue(branch3.getTimeZone().equals("GMT+01:00"));

        branch4 = m_branchManager.getBranch("branch40");
        assertEquals("Africa/Accra", branch4.getTimeZone());

        branch5 = m_branchManager.getBranch("branch50");
        assertTrue(branch5.getTimeZone().equals("GMT+01:00"));

        EasyMock.verify(setupManager, ntpManager);

    }

    public void testReplicateUserWithBranch() throws Exception {
        loadDataSet("branch/attached_branches.db.xml");
        Branch branch1 = m_branchManager.getBranch("branch1");
        assertNotNull(branch1);

        User u = m_coreContext.loadUser(1000);
        assertSame(branch1, u.getBranch());
        assertSame(branch1, m_locationManager.getLocation(1000).getBranch());

        m_coreContext.saveUser(u);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1000", MongoConstants.USER_LOCATION, "branch1");

        u.setBranch(null);
        m_coreContext.saveUser(u);
        assertObjectWithIdFieldValueNotPresent(getEntityCollection(), "User1000", MongoConstants.USER_LOCATION, "branch1");

        u.setBranch(branch1);
        m_coreContext.saveUser(u);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1000", MongoConstants.USER_LOCATION, "branch1");
    }
}
