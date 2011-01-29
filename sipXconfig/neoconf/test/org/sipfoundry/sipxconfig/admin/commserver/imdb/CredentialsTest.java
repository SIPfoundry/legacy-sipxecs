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
import java.util.Iterator;

import org.sipfoundry.sipxconfig.admin.authcode.AuthCode;
import org.sipfoundry.sipxconfig.admin.authcode.AuthCodeManager;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeer;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeerManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;

import com.mongodb.DBCursor;
import com.mongodb.DBObject;

public class CredentialsTest extends MongoTestCase {
    private Credentials m_credentials;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        CoreContext coreContext = getCoreContext();
        CallGroupContext m_callGroupContext;
        m_credentials = new Credentials();
        coreContext.loadInternalUsers();
        expectLastCall().andReturn(Collections.EMPTY_LIST).anyTimes();
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.EMPTY_LIST).anyTimes();

        m_callGroupContext = createMock(CallGroupContext.class);
        m_callGroupContext.getCallGroups();
        expectLastCall().andReturn(Collections.EMPTY_LIST).anyTimes();

        for (SpecialUserType u : SpecialUserType.values()) {
            coreContext.getSpecialUserAsSpecialUser(u);
            expectLastCall().andReturn(new SpecialUser(u));
        }

        TlsPeer peer = new TlsPeer();
        InternalUser user = new InternalUser();
        user.setSipPassword("123");
        user.setPintoken("11");
        peer.setInternalUser(user);
        TlsPeerManager tlsPeerManager = createMock(TlsPeerManager.class);
        tlsPeerManager.getTlsPeers();
        expectLastCall().andReturn(Collections.EMPTY_LIST);

        AuthCode code = new AuthCode();
        code.setInternalUser(user);
        AuthCodeManager authCodeManager = createMock(AuthCodeManager.class);
        authCodeManager.getAuthCodes();
        expectLastCall().andReturn(Collections.EMPTY_LIST);

        replay(coreContext, m_callGroupContext, tlsPeerManager, authCodeManager);

        m_credentials.setCoreContext(coreContext);
        m_credentials.setCallGroupContext(m_callGroupContext);
        m_credentials.setDbCollection(getCollection());
        m_credentials.setTlsPeerManager(tlsPeerManager);
        m_credentials.setAuthCodeManager(authCodeManager);
    }

    public void testGenerateEmpty() throws Exception {

        m_credentials.generate();

        assertEquals(SpecialUserType.values().length, getCollection().find().count());
        DBCursor curr = getCollection().find();

        Iterator<DBObject> iter = curr.iterator();
        DBObject item = iter.next();
        assertEquals(Credentials.DIGEST, item.get(Credentials.AUTHTYPE));
        assertEquals("~~id~park", item.get("id"));
        assertEquals(DOMAIN, item.get(Credentials.REALM));
        for (SpecialUserType u : SpecialUserType.values()) {
            assertObjectWithIdPresent(u.getUserName());
        }
    }

    public void testAddCallgroup() throws Exception {
        CallGroup cg = new CallGroup();
        cg.setUniqueId(1);
        cg.setName("sales");
        cg.setSipPassword("pass4321");

        m_credentials.generate(cg);

        // Md5Encoder.digestPassword("sales", "sipfoundry.org", "pass4321");
        String digest = "8e5d70cc7173bca2802dd45113229c1b";

        assertObjectWithIdPresent("CallGroup1");
        assertObjectWithIdFieldValuePresent("CallGroup1", Credentials.IDENTITY, "sip:sales@" + DOMAIN);
        assertObjectWithIdFieldValuePresent("CallGroup1", Credentials.PASSTOKEN, "pass4321");
        assertObjectWithIdFieldValuePresent("CallGroup1", Credentials.PINTOKEN, digest);
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

        assertObjectWithIdPresent("User1");
        assertObjectWithIdFieldValuePresent("User1", Credentials.IDENTITY, "sip:superadmin@" + DOMAIN);
        assertObjectWithIdFieldValuePresent("User1", Credentials.PASSTOKEN, "pass4321");
        assertObjectWithIdFieldValuePresent("User1", Credentials.PINTOKEN,
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

        assertObjectWithIdPresent("User1");
        assertObjectWithIdFieldValuePresent("User1", Credentials.IDENTITY, "sip:superadmin@" + DOMAIN);
        String emptyHash = Md5Encoder.digestPassword("superadmin", DOMAIN, "");
        assertObjectWithIdFieldValuePresent("User1", Credentials.PASSTOKEN, "");
        assertObjectWithIdFieldValuePresent("User1", Credentials.PINTOKEN, emptyHash);
        assertObjectWithIdFieldValuePresent("User1", Credentials.REALM, DOMAIN);
    }

}
