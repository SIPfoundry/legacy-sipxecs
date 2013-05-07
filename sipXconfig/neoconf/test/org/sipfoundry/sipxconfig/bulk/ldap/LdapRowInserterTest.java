/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import javax.naming.directory.Attribute;
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchResult;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.sipfoundry.sipxconfig.bulk.RowInserter.RowStatus;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

public class LdapRowInserterTest extends TestCase {
    private static final String JOE = "joe";
    private static final String SALES = "sales";
    private static final String LDAP = "ldap";
    private static final String NO_LDAP = "noldap";

    private LdapRowInserter m_rowInserter;

    @Override
    protected void setUp() {
        m_rowInserter = new LdapRowInserter();
        AttrMap attrMap = new AttrMap();
        attrMap.setDefaultGroupName("test-import");
        attrMap.setAttribute(Index.USERNAME.getName(), "identity");
        m_rowInserter.setAttrMap(attrMap);
    }

    private User insertRow(boolean existingUser) throws Exception {
        IMocksControl control = org.easymock.classextension.EasyMock.createNiceControl();
        UserMapper userMapper = control.createMock(UserMapper.class);
        SearchResult searchResult = control.createMock(SearchResult.class);
        Attributes attributes = control.createMock(Attributes.class);

        userMapper.getUserName(attributes);
        control.andReturn(JOE);
        userMapper.getGroupNames(searchResult);
        control.andReturn(Collections.singleton(SALES));
        control.replay();

        User joe = new User();
        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        joe.setPermissionManager(pManager);
        Group salesGroup = new Group();
        salesGroup.setName(SALES);
        salesGroup.setUniqueId();
        Group importGroup = new Group();
        importGroup.setName("import");
        importGroup.setUniqueId();

        AttrMap map = new AttrMap();
        map.setObjectClass("person");
        map.setDefaultGroupName("test-import");

        IMocksControl coreContextControl = EasyMock.createControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        LdapManager ldapManager = coreContextControl.createMock(LdapManager.class);
        coreContext.getGroupByName("test-import", false);
        coreContextControl.andReturn(importGroup);

        coreContext.loadUserByUserName(JOE);
        if (!existingUser) {
            coreContextControl.andReturn(null);
            coreContext.newUser();
            coreContextControl.andReturn(joe);
        } else {
            Group ldapGroup = new Group();
            ldapGroup.setName(LDAP);
            ldapGroup.setUniqueId();
            ldapGroup.setSettingValue(LdapRowInserter.LDAP_SETTING, "true");
            Group noLdapGroup = new Group();
            noLdapGroup.setUniqueId();
            noLdapGroup.setName(NO_LDAP);
            Set<Group> groups = new HashSet<Group>();
            groups.add(salesGroup);
            groups.add(ldapGroup);
            groups.add(noLdapGroup);
            joe.setGroups(groups);
            coreContextControl.andReturn(joe);
        }

        coreContext.getGroupMembersNames(importGroup);
        coreContextControl.andReturn(Collections.singleton("olderImportUser"));

        coreContext.getGroupByName(SALES, true);
        coreContextControl.andReturn(salesGroup);
        coreContext.saveUser(joe);
        coreContextControl.andReturn(true).atLeastOnce();
        coreContext.deleteUsersByUserName(Collections.singleton("olderImportUser"));
        ldapManager.retriveOverwritePin();
        coreContextControl.andReturn(new OverwritePinBean(100, true)).anyTimes();
        coreContextControl.replay();

        IMocksControl mailboxManagerControl = EasyMock.createControl();
        MailboxManager mailboxManager = mailboxManagerControl.createMock(MailboxManager.class);
        mailboxManager.isEnabled();
        mailboxManagerControl.andReturn(true);
        mailboxManager.deleteMailbox(JOE);
        mailboxManagerControl.replay();

        UserMapper rowInserterUserMapper = new UserMapper();
        m_rowInserter.setLdapManager(ldapManager);
        m_rowInserter.setUserMapper(rowInserterUserMapper);
        m_rowInserter.setCoreContext(coreContext);
        m_rowInserter.setUserMapper(userMapper);
        m_rowInserter.setMailboxManager(mailboxManager);
        m_rowInserter.setAttrMap(map);
        m_rowInserter.setDomain("example.com");
        m_rowInserter.beforeInserting(null);
        m_rowInserter.insertRow(searchResult, attributes);
        m_rowInserter.afterInserting();

        coreContextControl.verify();
        control.verify();
        return joe;
    }

    public void testInsertRowExistingUser() throws Exception {
        User joe = insertRow(true);
        assertEquals("example.com", joe.getSettingValue(User.DOMAIN_SETTING));
        assertEquals(2, joe.getGroups().size());
        //existing ldap group have been deleted and replaced with new ldap group
        for (Group group : joe.getGroups()) {
            if (StringUtils.equals(group.getName(), SALES)) {
                assertTrue(new Boolean(group.getSettingValue(LdapRowInserter.LDAP_SETTING)));
            } else if (StringUtils.equals(group.getName(), NO_LDAP)) {
                assertFalse(new Boolean(group.getSettingValue(LdapRowInserter.LDAP_SETTING)));
            }
        }
    }

    public void testInsertRowNewUser() throws Exception {
        User joe = insertRow(false);
        assertEquals(1, joe.getGroups().size());
        //ldap group was saved
        for (Group group : joe.getGroups()) {
            if (StringUtils.equals(group.getName(), SALES)) {
                assertTrue(new Boolean(group.getSettingValue(LdapRowInserter.LDAP_SETTING)));
            }
        }
    }

    public void testCheckRowData() throws Exception {
        IMocksControl control = org.easymock.classextension.EasyMock.createNiceControl();
        UserMapper userMapper = control.createMock(UserMapper.class);
        SearchResult searchResult = control.createMock(SearchResult.class);
        Attributes attributes = control.createMock(Attributes.class);
        Attribute attribute = control.createMock(Attribute.class);

        searchResult.getAttributes();
        control.andReturn(attributes);
        attributes.get("identity");
        control.andReturn(null);
        control.replay();
        m_rowInserter.setUserMapper(userMapper);
        assertEquals(RowStatus.FAILURE, m_rowInserter.checkRowData(searchResult).getRowStatus());

        control.reset();
        searchResult.getAttributes();
        control.andReturn(attributes);
        attributes.get("identity");
        control.andReturn(attribute);
        userMapper.getUserName(attributes);
        control.andReturn("@McQueen");
        userMapper.getGroupNames(searchResult);
        control.andReturn(Collections.singleton(SALES));
        control.replay();
        m_rowInserter.setUserMapper(userMapper);
        assertEquals(RowStatus.FAILURE, m_rowInserter.checkRowData(searchResult).getRowStatus());

        control.reset();
        searchResult.getAttributes();
        control.andReturn(attributes);
        attributes.get("identity");
        control.andReturn(attribute);
        userMapper.getUserName(attributes);
        control.andReturn("McQueen");
        userMapper.getGroupNames(searchResult);
        control.andReturn(Collections.singleton(SALES));
        control.replay();
        m_rowInserter.setUserMapper(userMapper);
        assertEquals(RowStatus.SUCCESS, m_rowInserter.checkRowData(searchResult).getRowStatus());
    }
}
