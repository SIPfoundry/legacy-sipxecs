package org.sipfoundry.sipxconfig.bulk.ldap;

import javax.naming.NamingException;
import javax.naming.directory.Attributes;
import javax.naming.directory.BasicAttributes;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;

public class UserMapperTest extends TestCase {
    private UserMapper m_userMapper;
    private User m_user;

    @Override
    protected void setUp() {
        m_user = new User();
        m_userMapper = new UserMapper();
        AttrMap attrMap = new AttrMap();
        attrMap.setAttribute("userName", "uid");
        attrMap.setAttribute("firstName","firstName");
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
        attrs.put("imId", "200_tester");
        attrs.put("job", "engineer");
        attrs.put("homeStreet", "Route66");
        attrs.put("officeCity", "Boston");
        try {
            m_userMapper.setUserProperties(m_user, attrs);
            assertEquals("200", m_user.getUserName());
            assertEquals("tester", m_user.getFirstName());
            assertEquals("200_tester", m_user.getImId());
            assertEquals("engineer", m_user.getAddressBookEntry().getJobTitle());
            assertEquals("Route66", m_user.getAddressBookEntry().getHomeAddress().getStreet());
            assertEquals("Boston", m_user.getAddressBookEntry().getOfficeAddress().getCity());
        } catch(NamingException ex) {
            fail();
        }
    }
}
