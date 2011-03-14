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

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.ArrayList;
import java.util.List;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;

public class CallerAliasesTest extends MongoTestCase {
    private final String[][] USER_DATA = {
        {
            "1", "first", "last", "userName", "1234", "ss"
        }, {
            "2", null, null, "kuku", "4321", "87654321"
        }, {
            "3", "user", "without", "number", null, "12345"
        },
    };

    private final Object[][] GATEWAY_DATA = {
        {
            1, "example.org", 0, "7832331111", true, false, "", -1, false, false, 1
        }, {
            2, "bongo.com", 5060, null, false, false, "", -1, false, false, 2
        }, {
            3, "kuku.net", 1025, "233", false, true, "+", 2, true, false, 3
        }, {
            4, "1.2.3.4", 0, "1234", false, false, "", -1, false, true, 4
        }
    };

    private List<User> m_users;

    private List<Gateway> m_gateways;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        PermissionManagerImpl impl = new PermissionManagerImpl();
        impl.setModelFilesContext(TestHelper.getModelFilesContext());
        DomainManager dm = getDomainManager();
        replay(dm);
        m_users = new ArrayList<User>();
        for (String[] ud : USER_DATA) {
            User user = new User();
            user.setPermissionManager(impl);
            user.setUniqueId(new Integer(ud[0]));
            user.setFirstName(ud[1]);
            user.setLastName(ud[2]);
            user.setUserName(ud[3]);
            user.setSettingValue(UserCallerAliasInfo.EXTERNAL_NUMBER, ud[4]);
            user.addAlias(ud[5]);
            user.setDomainManager(dm);
            m_users.add(user);
        }

        m_gateways = new ArrayList<Gateway>();
        for (Object[] gd : GATEWAY_DATA) {
            Gateway gateway = new Gateway();
            gateway.setUniqueId((Integer) gd[0]);
            gateway.setAddress((String) gd[1]);
            gateway.setAddressPort((Integer) gd[2]);
            GatewayCallerAliasInfo info = new GatewayCallerAliasInfo();
            info.setDefaultCallerAlias((String) gd[3]);
            info.setIgnoreUserInfo((Boolean) gd[4]);
            info.setTransformUserExtension((Boolean) gd[5]);
            info.setAddPrefix((String) gd[6]);
            info.setKeepDigits((Integer) gd[7]);
            info.setAnonymous((Boolean) gd[8]);
            info.setEnableCallerId((Boolean) gd[9]);
            gateway.setCallerAliasInfo(info);
            gateway.setUniqueId(((Integer) gd[10]).intValue());
            m_gateways.add(gateway);
        }
    }

    public void testGenerate() throws Exception {
        CallerAliases cas = new CallerAliases();

        cas.setAnonymousAlias("sip:anonymous@anonymous.invalid");

        CoreContext coreContext = getCoreContext();
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(m_users).anyTimes();
        replay(coreContext);

        GatewayContext gatewayContext = EasyMock.createMock(GatewayContext.class);
        gatewayContext.getGateways();
        expectLastCall().andReturn(m_gateways).anyTimes();
        replay(gatewayContext);

        cas.setCoreContext(coreContext);
        cas.setGatewayContext(gatewayContext);
        cas.setDbCollection(getCollection());

        cas.generate(m_users.get(0));
        cas.generate(m_users.get(1));
        cas.generate(m_users.get(2));
        cas.generate(m_gateways.get(0));
        cas.generate(m_gateways.get(1));
        cas.generate(m_gateways.get(2));
        cas.generate(m_gateways.get(3));
        
        MongoTestCaseHelper.assertCollectionCount(6);
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("Gateway1", CallerAliases.CALLERALIASES + "."
                + CallerAliasesMapping.DOMAIN, "example.org;sipxecs-lineid=1");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("Gateway1", CallerAliases.CALLERALIASES + "."
                + CallerAliasesMapping.ALIAS, "sip:7832331111@" + DOMAIN);
        MongoTestCaseHelper.assertObjectWithIdNotPresent("Gateway2");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("Gateway3", CallerAliases.CALLERALIASES + "."
                + CallerAliasesMapping.DOMAIN, "kuku.net:1025;sipxecs-lineid=3");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("Gateway3", CallerAliases.CALLERALIASES + "."
                + CallerAliasesMapping.ALIAS, "sip:anonymous@anonymous.invalid");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("Gateway4", CallerAliases.CALLERALIASES + "."
                + CallerAliasesMapping.DOMAIN, "1.2.3.4;sipxecs-lineid=4");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("Gateway4", CallerAliases.CALLERALIASES + "."
                + CallerAliasesMapping.ALIAS, "sip:1234@" + DOMAIN);
        // users
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1",
                CallerAliases.CALLERALIASES + "." + CallerAliasesMapping.DOMAIN, "bongo.com:5060;sipxecs-lineid=2");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1", CallerAliases.CALLERALIASES + "." + CallerAliasesMapping.ALIAS,
                "\"first last\"<sip:1234@" + DOMAIN + ">");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1",
                CallerAliases.CALLERALIASES + "." + CallerAliasesMapping.DOMAIN, "kuku.net:1025;sipxecs-lineid=3");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User2",
                CallerAliases.CALLERALIASES + "." + CallerAliasesMapping.DOMAIN, "bongo.com:5060;sipxecs-lineid=2");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User2", CallerAliases.CALLERALIASES + "." + CallerAliasesMapping.ALIAS,
                "sip:4321@" + DOMAIN);
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User2",
                CallerAliases.CALLERALIASES + "." + CallerAliasesMapping.DOMAIN, "kuku.net:1025;sipxecs-lineid=3");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User2", CallerAliases.CALLERALIASES + "." + CallerAliasesMapping.ALIAS,
                "sip:+21@" + DOMAIN);
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User3",
                CallerAliases.CALLERALIASES + "." + CallerAliasesMapping.DOMAIN, "kuku.net:1025;sipxecs-lineid=3");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User3", CallerAliases.CALLERALIASES + "." + CallerAliasesMapping.ALIAS,
                "\"user without\"<sip:+45@" + DOMAIN + ">");
    }
}
