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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class UserStaticTest extends XMLTestCase {
    public UserStaticTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    private final String[][] USER_DATA = {
        {
            "first1", "last1", "8809", "63948809"
        }, {
            "first2", "last2", "8810", "63948810"
        }, {
            "first3", "last3", "8811", "63948811"
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
            user.setSettingValue("voicemail/mailbox/external-mwi", ud[3]);
            m_users.add(user);
        }
    }

    public void testGenerateEmpty() throws Exception {
        UserStatic us = new UserStatic() {
            @Override
            protected String getSipDomain() {
                return "example.org";
            }
        };

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.emptyList());

        us.setCoreContext(coreContext);

        replay(coreContext);

        List<Map<String, String>> document = us.generate();
        assertEquals(0, document.size());

        verify(coreContext);
    }

    public void testGenerate() throws Exception {
        UserStatic us = new UserStatic() {
            @Override
            protected String getSipDomain() {
                return "example.org";
            }
        };

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(m_users);
        us.setCoreContext(coreContext);
        replay(coreContext);

        Document document = us.generateXml();
        String usXml = XmlUnitHelper.asString(document);

        InputStream referenceXmlStream = UserStaticTest.class.getResourceAsStream("userstatic.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(usXml));
    }
}
