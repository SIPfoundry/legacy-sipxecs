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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.Collection;
import java.util.Collections;
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
    private LdapManager m_ldapManager;
    private User m_user;

    @Override
    protected void setUp() {
        m_user = new User();
        m_userMapper = new UserMapper();
        m_ldapManager = createMock(LdapManager.class);
        m_ldapManager.retriveOverwritePin();
        expectLastCall().andReturn(new OverwritePinBean(10, true)).anyTimes();
        replay(m_ldapManager);
        AttrMap attrMap = new AttrMap();
        m_userMapper.setLdapManager(m_ldapManager);
        attrMap.setAttribute("userName", "uid");
        attrMap.setAttribute("firstName","firstName");
        attrMap.setAttribute("aliasesString", "telephoneNumber");
        attrMap.setAttribute("imId", "imId");
        attrMap.setAttribute("userProfile.jobTitle","job");
        attrMap.setAttribute("userProfile.homeAddress.street", "homeStreet");
        attrMap.setAttribute("userProfile.officeAddress.city", "officeCity");
        m_userMapper.setAttrMap(attrMap);
    }

    public void testVoicemailPin() {
        User user = new User();
        Attributes attrs = new BasicAttributes();
        attrs.put("uid", "200");
        attrs.put("vmpin", "1234");
        attrs.put("vmpin2", "1235");
        attrs.put("vmpin3", "1236");
        UserMapper userMapper = new UserMapper();
        userMapper.setLdapManager(m_ldapManager);
        AttrMap attrMap = new AttrMap();
        attrMap.setAttribute("userName", "uid");
        attrMap.setDefaultPin("5555");
        userMapper.setAttrMap(attrMap);
        try {
            //user is not new
            userMapper.setUserProperties(user, attrs);

            user.setUniqueId(1);
            assertEquals("200", user.getUserName());

            attrMap.setAttribute("voicemailPin", "vmpin");
            userMapper.setVoicemailPin(user, attrs);
            assertEquals("2331a3a7bce8433908996c6f9f71f2fe", user.getVoicemailPintoken());

            attrMap.setAttribute("voicemailPin", "");
            userMapper.setVoicemailPin(user, attrs);
            assertEquals("2331a3a7bce8433908996c6f9f71f2fe", user.getVoicemailPintoken());

            attrMap.setAttribute("voicemailPin", "vmpin2");
            userMapper.setVoicemailPin(user, attrs);
            assertEquals("d04f6bd78f3e78d9110b029b5d932b72", user.getVoicemailPintoken());

            //user is new
            user.setUniqueId(-1);

            attrMap.setAttribute("voicemailPin", "vmpin3");
            userMapper.setVoicemailPin(user, attrs);
            assertEquals("bac667f0f8976efe1fc042aefdf5acab", user.getVoicemailPintoken());

            //user is new, no mapping pin found => the default PIN is set
            attrMap.setAttribute("voicemailPin", "");
            userMapper.setVoicemailPin(user, attrs);
            assertEquals("4646f2ec919ec583df43e4087fe66dd2", user.getVoicemailPintoken());
        } catch (NamingException e) {
            fail();
        }
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
            assertEquals("engineer", m_user.getUserProfile().getJobTitle());
            assertEquals("Route66", m_user.getUserProfile().getHomeAddress().getStreet());
            assertEquals("Boston", m_user.getUserProfile().getOfficeAddress().getCity());
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
