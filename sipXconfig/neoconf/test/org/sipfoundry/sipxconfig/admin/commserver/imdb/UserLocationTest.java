/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class UserLocationTest extends XMLTestCase {
    public UserLocationTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    private final String[][] USER_DATA = {
        {
            "first1", "last1", "mir1", "boston"
        }, {
            "first2", ",last2", "mir2", "seattle"
        }, {
            "first3", ",last3", "mir3", null
        },
    };

    private List<User> m_users;

    @Override
    protected void setUp() throws Exception {
        PermissionManagerImpl impl = new PermissionManagerImpl();
        impl.setModelFilesContext(TestHelper.getModelFilesContext());

        m_users = new ArrayList<User>();
        for (String[] ud : USER_DATA) {
            User user = new User();
            user.setPermissionManager(impl);

            user.setFirstName(ud[0]);
            user.setLastName(ud[1]);
            user.setUserName(ud[2]);
            if (ud[3] != null) {
                Branch branch = new Branch();
                branch.setUniqueId();
                branch.setName(ud[3]);

                Group site = new Group();
                site.setUniqueId();
                site.setName("group" + ud[3]);
                site.setBranch(branch);

                user.addGroup(site);
            }
            m_users.add(user);
        }
    }

    public void testGenerateEmpty() throws Exception {
        UserLocation ul = new UserLocation() {
            @Override
            protected String getSipDomain() {
                return "example.org";
            }
        };

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.emptyList());

        ul.setCoreContext(coreContext);

        replay(coreContext);

        List<Map<String, String>> document = ul.generate();
        assertEquals(0, document.size());

        verify(coreContext);
    }

    public void testGenerate() throws Exception {
        UserLocation ul = new UserLocation() {
            @Override
            protected String getSipDomain() {
                return "example.org";
            }
        };

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(m_users);

        ul.setCoreContext(coreContext);

        replay(coreContext);

        Document document = ul.generateXml();
        String ulXml = TestUtil.asString(document);

        InputStream referenceXmlStream = UserLocationTest.class.getResourceAsStream("userlocation.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(ulXml));

        verify(coreContext);
    }
}
