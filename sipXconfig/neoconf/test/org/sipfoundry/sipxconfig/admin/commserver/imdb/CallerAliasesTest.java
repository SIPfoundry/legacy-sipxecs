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
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class CallerAliasesTest extends XMLTestCase {
    public CallerAliasesTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    private final String[][] USER_DATA = {
        {
            "first", "last", "userName", "1234", "ss"
        }, {
            null, null, "kuku", "4321", "87654321"
        }, {
            "user", "without", "number", null, "12345"
        },
    };

    private final Object[][] GATEWAY_DATA = {
        {
            "example.org", 0, "7832331111", true, false, "", -1, false, false, 1
        }, {
            "bongo.com", 5060, null, false, false, "", -1, false, false, 2
        }, {
            "kuku.net", 1025, "233", false, true, "+", 2, true, false, 3
        }, {
            "1.2.3.4", 0, "1234", false, false, "", -1, false, true, 4
        }
    };

    private List<User> m_users;

    private List<Gateway> m_gateways;

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
            user.setSettingValue(UserCallerAliasInfo.EXTERNAL_NUMBER, ud[3]);
            user.addAlias(ud[4]);
            m_users.add(user);
        }

        m_gateways = new ArrayList<Gateway>();
        for (Object[] gd : GATEWAY_DATA) {
            Gateway gateway = new Gateway();
            gateway.setAddress((String) gd[0]);
            gateway.setAddressPort((Integer) gd[1]);
            GatewayCallerAliasInfo info = new GatewayCallerAliasInfo();
            info.setDefaultCallerAlias((String) gd[2]);
            info.setIgnoreUserInfo((Boolean) gd[3]);
            info.setTransformUserExtension((Boolean) gd[4]);
            info.setAddPrefix((String) gd[5]);
            info.setKeepDigits((Integer) gd[6]);
            info.setAnonymous((Boolean) gd[7]);
            info.setEnableCallerId((Boolean) gd[8]);
            gateway.setCallerAliasInfo(info);
            gateway.setUniqueId(((Integer)gd[9]).intValue());
            m_gateways.add(gateway);
        }
    }

    public void testGenerateEmpty() throws Exception {
        CallerAliases cas = new CallerAliases() {
            @Override
            protected String getSipDomain() {
                return "example.org";
            }
        };

        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        EasyMock.expectLastCall().andReturn(Collections.emptyList()).anyTimes();

        GatewayContext gatewayContext = EasyMock.createMock(GatewayContext.class);
        gatewayContext.getGateways();
        EasyMock.expectLastCall().andReturn(Collections.emptyList());

        cas.setCoreContext(coreContext);
        cas.setGatewayContext(gatewayContext);

        EasyMock.replay(coreContext, gatewayContext);

        List<Map<String, String>> document = cas.generate();
        assertEquals(0, document.size());

        EasyMock.verify(coreContext, gatewayContext);
    }

    public void testGenerate() throws Exception {
        CallerAliases cas = new CallerAliases() {
            @Override
            protected String getSipDomain() {
                return "example.org";
            }
        };

        cas.setAnonymousAlias("sip:anonymous@anonymous.invalid");

        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        EasyMock.expectLastCall().andReturn(m_users).anyTimes();

        GatewayContext gatewayContext = EasyMock.createMock(GatewayContext.class);
        gatewayContext.getGateways();
        EasyMock.expectLastCall().andReturn(m_gateways);

        cas.setCoreContext(coreContext);
        cas.setGatewayContext(gatewayContext);

        EasyMock.replay(coreContext, gatewayContext);

        Document document = cas.generateXml();
        String casXml = TestUtil.asString(document);

        InputStream referenceXmlStream = AliasesTest.class.getResourceAsStream("caller-alias.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(casXml));

        EasyMock.verify(coreContext, gatewayContext);
    }
}
