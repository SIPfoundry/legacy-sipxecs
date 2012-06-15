/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Collection;
import java.util.Set;

import javax.naming.NamingException;
import javax.naming.directory.Attributes;
import javax.naming.directory.BasicAttributes;
import javax.naming.directory.SearchResult;

import junit.framework.TestCase;

import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.common.User;

public class UserMapperTest extends TestCase {
    private static final String LDAP_GROUP = "ldapGroup";
    private static final String LDAP_1_GROUP = "ldap1Group";
    private static final String LDAP_IMPORTS = "ldap_imports";
    private UserMapper m_userMapper;
    private User m_user;

    @Override
    protected void setUp() {
        m_user = new User();
        m_userMapper = new UserMapper();
        AttrMap attrMap = new AttrMap();
        attrMap.setAttribute("userName", "uid");
        attrMap.setAttribute("firstName","firstName");
        attrMap.setAttribute("aliasesString", "telephoneNumber");
        attrMap.setAttribute("imId", "imId");
        attrMap.setAttribute("createdAddressBookEntry.jobTitle","job");
        attrMap.setAttribute("createdAddressBookEntry.homeAddress.street", "homeStreet");
        attrMap.setAttribute("createdAddressBookEntry.officeAddress.city", "officeCity");
        m_userMapper.setAttrMap(attrMap);
    }

    public void testSetProperties() {
        Attributes attrs = new BasicAttributes();
        attrs.put("uid", "200");
        attrs.put("firstName", "tester");
        attrs.put("telephoneNumber", "555 123 456");
        attrs.put("imId", "200_tester");
        attrs.put("job", "engineer");
        attrs.put("homeStreet", "Route66");
        attrs.put("officeCity", "Boston");
        try {
            m_userMapper.setUserProperties(m_user, attrs);
            Set<String> aliasesSet = m_userMapper.getAliasesSet(attrs);
            m_userMapper.setAliasesSet(aliasesSet, m_user);
            assertEquals("200", m_user.getUserName());
            assertEquals("tester", m_user.getFirstName());
            assertEquals("555123456", m_user.getAliasesString());
            assertEquals("200_tester", m_user.getImId());
            assertEquals("engineer", m_user.getAddressBookEntry().getJobTitle());
            assertEquals("Route66", m_user.getAddressBookEntry().getHomeAddress().getStreet());
            assertEquals("Boston", m_user.getAddressBookEntry().getOfficeAddress().getCity());
        } catch(NamingException ex) {
            fail();
        }
    }

    private Collection<String> getGroupNames(boolean existLdapGroup) throws Exception{
        Attributes attrs = new BasicAttributes();
        attrs.put(LDAP_GROUP, LDAP_1_GROUP);

        UserMapper userMapper = new UserMapper();

        IMocksControl control = org.easymock.classextension.EasyMock.createNiceControl();
        AttrMap map = control.createMock(AttrMap.class);
        SearchResult sr = control.createMock(SearchResult.class);

        map.userProperty2ldapAttribute(Index.USER_GROUP.getName());
        if (existLdapGroup) {
            control.andReturn(LDAP_GROUP);
        } else {
            control.andReturn(null);
            map.getDefaultGroupName();
            control.andReturn(LDAP_IMPORTS);
        }
        sr.getAttributes();
        control.andReturn(attrs);
        sr.isRelative();
        control.andReturn(false);
        control.replay();

        userMapper.setAttrMap(map);

        Collection<String> groups = userMapper.getGroupNames(sr);

        control.verify();
        return groups;
    }

    public void testLdapGroup() throws Exception {
        Collection<String> groups = getGroupNames(true);
        assertEquals(1, groups.size());
        assertEquals(LDAP_1_GROUP, groups.iterator().next());
    }

    public void testDefaultLdapGroup() throws Exception {
        Collection<String> groups = getGroupNames(false);
        assertEquals(1, groups.size());
        assertEquals(LDAP_IMPORTS, groups.iterator().next());
    }
}
