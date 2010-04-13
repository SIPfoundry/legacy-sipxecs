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

import java.util.Collections;

import javax.naming.directory.Attributes;
import javax.naming.directory.SearchResult;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

public class LdapRowInserterTest extends TestCase {
    private LdapRowInserter m_rowInserter;

    protected void setUp() {
        m_rowInserter = new LdapRowInserter();
        AttrMap attrMap = new AttrMap();
        attrMap.setDefaultGroupName("test-import");
        m_rowInserter.setAttrMap(attrMap);
    }

    public void testInsertRow() throws Exception {
        IMocksControl control = org.easymock.classextension.EasyMock.createNiceControl();
        UserMapper userMapper = control.createMock(UserMapper.class);
        SearchResult searchResult = control.createMock(SearchResult.class);
        Attributes attributes = control.createMock(Attributes.class);

        userMapper.getUserName(attributes);
        control.andReturn("joe");
        userMapper.getGroupNames(searchResult);
        control.andReturn(Collections.singleton("sales"));
        control.replay();

        User joe = new User();
        Group salesGroup = new Group();
        Group importGroup = new Group();

        AttrMap map = new AttrMap();
        map.setObjectClass("person");

        IMocksControl coreContextControl = EasyMock.createControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        LdapManager ldapManager = coreContextControl.createMock(LdapManager.class);
        ldapManager.getAttrMap();
        coreContextControl.andReturn(map);
        coreContext.getGroupByName("test-import", false);
        coreContextControl.andReturn(importGroup);
        coreContext.loadUserByUserName("joe");

        // another useful test would be to return an existing user
        coreContextControl.andReturn(null);

        coreContext.getGroupMembersNames(importGroup);
        coreContextControl.andReturn(Collections.singleton("olderImportUser"));
        coreContext.newUser();
        coreContextControl.andReturn(joe);
        coreContext.getGroupByName("sales", true);
        coreContextControl.andReturn(salesGroup);
        coreContext.saveUser(joe);
        coreContextControl.andReturn(true).atLeastOnce();
        coreContext.deleteUsersByUserName(Collections.singleton("olderImportUser"));
        coreContextControl.replay();

        IMocksControl mailboxManagerControl = EasyMock.createControl();
        MailboxManager mailboxManager = mailboxManagerControl.createMock(MailboxManager.class);
        mailboxManager.isEnabled();
        mailboxManagerControl.andReturn(true);
        mailboxManager.deleteMailbox("joe");
        mailboxManagerControl.replay();

        UserMapper rowInserterUserMapper = new UserMapper();
        m_rowInserter.setLdapManager(ldapManager);
        m_rowInserter.setUserMapper(rowInserterUserMapper);
        m_rowInserter.setCoreContext(coreContext);
        m_rowInserter.setUserMapper(userMapper);
        m_rowInserter.setMailboxManager(mailboxManager);
        m_rowInserter.beforeInserting();
        m_rowInserter.insertRow(searchResult, attributes);
        m_rowInserter.afterInserting();

        coreContextControl.verify();
        control.verify();
    }
}
