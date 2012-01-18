/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.IOException;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class SettingDaoTestIntegration extends IntegrationTestCase {
    private CoreContext m_coreContext;
    private BranchManager m_branchManager;
    private SettingDao m_settingDao;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testSettingGroup() throws Exception {
        ValueStorage vs = new ValueStorage();
        vs.setSettingValue(new SettingImpl("setting"), new SettingValueImpl("some value"), null);
        m_settingDao.storeValueStorage(vs);
    }

    public void testGetGroupsByString() throws Exception {
        List<Group> groups = m_settingDao.getGroupsByString("foo", " g0 g1   g2 \t g3 \n g4 ", false);        
        assertEquals(5, groups.size());
        assertEquals("g0", groups.get(0).getName());
        assertEquals("g4", groups.get(4).getName());
        commit();
        assertEquals(0, countRowsInTable("group_storage"));
    }

    public void testGetGroupsByStringAndSave() throws Exception {
        List<Group> groups = m_settingDao.getGroupsByString("foo", " g0 g1   g2 \t g3 \n g4 ", true);
        assertEquals(5, groups.size());
        commit();
        assertEquals(5, countRowsInTable("group_storage"));
    }

    public void testGetGroupsByEmptyString() throws Exception {
        assertEquals(0, m_settingDao.getGroupsByString("foo", "", false).size());
        assertEquals(0, m_settingDao.getGroupsByString("foo", "  ", false).size());
        assertEquals(0, m_settingDao.getGroupsByString("foo", null, false).size());
    }

    public void testInvalidBranchGroup() throws Exception {
        sql("domain/DomainSeed.sql");
        sql("commserver/SeedLocations.sql");
        Branch branch1 = new Branch();
        branch1.setName("branch1");
        Branch branch2 = new Branch();
        branch2.setName("branch2");
        m_branchManager.saveBranch(branch1);
        m_branchManager.saveBranch(branch2);

        Group group = new Group();
        group.setName("group1");

        User user = m_coreContext.newUser();
        user.setFirstName("First");
        user.setLastName("Last");
        user.setBranch(branch1);
        user.addGroup(group);
        m_coreContext.saveUser(user);

        group.setBranch(branch2);
        try {
            m_settingDao.saveGroup(group);
            //should never be here
            fail();
        } catch (UserException ex) {

        }
    }

    public void testGetBranchMemberCountIndexedByBranchId() throws IOException {
        sql("setting/user-group-branch.sql");
        Map<Integer, Long> branchesMap = m_settingDao.getBranchMemberCountIndexedByBranchId(User.class);
        assertEquals(3, branchesMap.size());
        assertEquals(1, branchesMap.get(1000).longValue());
        assertEquals(2, branchesMap.get(1001).longValue());
        assertEquals(1, branchesMap.get(1002).longValue());
    }

    public void testGetGroupBranchMemberCountIndexedByBranchId() throws IOException {
        sql("setting/user-group-branch.sql");
        Map<Integer, Long> branchesMap = m_settingDao.getGroupBranchMemberCountIndexedByBranchId(User.class);
        assertEquals(2, branchesMap.size());
        assertEquals(1, branchesMap.get(1000).longValue());
        assertEquals(1, branchesMap.get(1001).longValue());
    }

    public void testGetAllBranchMemberCountIndexedByBranchId() throws IOException {
        sql("setting/user-group-branch.sql");
        Map<Integer, Long> branchesMap = m_settingDao.getAllBranchMemberCountIndexedByBranchId(User.class);
        assertEquals(3, branchesMap.size());
        assertEquals(2, branchesMap.get(1000).longValue());
        assertEquals(3, branchesMap.get(1001).longValue());
        assertEquals(1, branchesMap.get(1002).longValue());
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }
}
