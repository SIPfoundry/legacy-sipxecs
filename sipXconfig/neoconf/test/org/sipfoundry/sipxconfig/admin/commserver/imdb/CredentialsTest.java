/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.Collections;

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class CredentialsTest extends MongoTestCase {
    private Credentials m_credentials;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        CoreContext coreContext = getCoreContext();
        m_credentials = new Credentials();
        coreContext.loadInternalUsers();
        expectLastCall().andReturn(Collections.EMPTY_LIST).anyTimes();
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.EMPTY_LIST).anyTimes();

        replay(coreContext);

        m_credentials.setCoreContext(coreContext);
        m_credentials.setDbCollection(getCollection());
    }

    public void testAddCallgroup() throws Exception {
        CallGroup cg = new CallGroup();
        cg.setUniqueId(1);
        cg.setName("sales");
        cg.setSipPassword("pass4321");

        m_credentials.generate(cg);

        // Md5Encoder.digestPassword("sales", "sipfoundry.org", "pass4321");
        String digest = "8e5d70cc7173bca2802dd45113229c1b";

        MongoTestCaseHelper.assertObjectWithIdPresent("CallGroup1");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("CallGroup1", Credentials.IDENTITY, "sales@" + DOMAIN);
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("CallGroup1", Credentials.PASSTOKEN, "pass4321");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("CallGroup1", Credentials.PINTOKEN, digest);
    }

    public void testAddUser() throws Exception {
        DomainManager dm = getDomainManager();
        replay(dm);
        User user = new User();
        user.setUniqueId(1);
        user.setUserName("superadmin");
        final String PIN = "pin1234";
        user.setPin(PIN, DOMAIN);
        user.setSipPassword("pass4321");
        user.setDomainManager(dm);

        m_credentials.generate(user);

        MongoTestCaseHelper.assertObjectWithIdPresent("User1");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1", Credentials.IDENTITY, "superadmin@" + DOMAIN);
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1", Credentials.PASSTOKEN, "pass4321");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1", Credentials.PINTOKEN,
                Md5Encoder.digestPassword("superadmin", DOMAIN, PIN));
    }

    public void testAddUserEmptyPasswords() throws Exception {
        DomainManager dm = createMock(DomainManager.class);
        dm.getDomainName();
        expectLastCall().andReturn(DOMAIN).anyTimes();
        replay(dm);
        User user = new User();
        user.setUniqueId(1);
        user.setUserName("superadmin");
        user.setPin("", DOMAIN);
        user.setDomainManager(dm);

        m_credentials.generate(user);

        MongoTestCaseHelper.assertObjectWithIdPresent("User1");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1", Credentials.IDENTITY, "superadmin@" + DOMAIN);
        String emptyHash = Md5Encoder.digestPassword("superadmin", DOMAIN, "");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1", Credentials.PASSTOKEN, "");
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1", Credentials.PINTOKEN, emptyHash);
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1", Credentials.REALM, DOMAIN);
    }

}
