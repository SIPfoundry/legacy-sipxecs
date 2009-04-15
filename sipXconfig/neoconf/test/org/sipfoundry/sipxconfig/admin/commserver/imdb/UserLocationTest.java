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
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.setting.Group;

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
                Group site = new Group();
                site.setUniqueId();
                site.setName(ud[3]);
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

        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        coreContext.loadUsers();
        EasyMock.expectLastCall().andReturn(Collections.emptyList());

        ul.setCoreContext(coreContext);

        EasyMock.replay(coreContext);

        List<Map<String, String>> document = ul.generate();
        assertEquals(0, document.size());

        EasyMock.verify(coreContext);
    }

    public void testGenerate() throws Exception {
        UserLocation ul = new UserLocation() {
            @Override
            protected String getSipDomain() {
                return "example.org";
            }
        };

        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        coreContext.loadUsers();
        EasyMock.expectLastCall().andReturn(m_users);

        ul.setCoreContext(coreContext);

        EasyMock.replay(coreContext);

        Document document = ul.generateXml();
        String ulXml = XmlUnitHelper.asString(document);

        InputStream referenceXmlStream = UserLocationTest.class.getResourceAsStream("userlocation.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(ulXml));

        EasyMock.verify(coreContext);
    }
}
