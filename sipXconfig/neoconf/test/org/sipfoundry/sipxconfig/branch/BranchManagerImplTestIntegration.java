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

import java.util.Arrays;
import java.util.List;

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class BranchManagerImplTestIntegration extends IntegrationTestCase {

    private static final int NUM_BRANCHES = 5;

    private BranchManager m_branchManager;
    private CoreContext m_coreContext;
    private PhoneContext m_phoneContext;
    private LocationsManager m_locationManager;
    private SbcDeviceManager m_sbcDeviceManager;

    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }

    public void setLocationsManager(LocationsManager locationManager) {
        m_locationManager = locationManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
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
        m_branchManager.deleteBranch(branch1);
        assertNull(m_branchManager.getBranch("branch1"));
    }

    public void testDeleteAttachedBranch() throws Exception {
        loadDataSet("branch/attached_branches.db.xml");

        Branch branch1 = m_branchManager.getBranch("branch1");
        assertNotNull(branch1);

        assertSame(branch1, m_coreContext.loadUser(1000).getBranch());
        assertSame(branch1, m_phoneContext.loadPhone(1000).getBranch());
        assertSame(branch1, m_sbcDeviceManager.getSbcDevice(1000).getBranch());
        assertSame(branch1, m_locationManager.getLocation(1000).getBranch());

        m_branchManager.deleteBranch(branch1);

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
}
