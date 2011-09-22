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

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;

public class CredentialsIntegrationTest extends ImdbTestCase {
    private Credentials m_credentials;
    
    @Override
    protected void onSetUpBeforeTransaction() {
        // otherwise realm is null for password hash related calls.
        getDomainManager().initializeDomain();        
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_credentials = new Credentials();
        m_credentials.setCoreContext(getCoreContext());
        m_credentials.setDbCollection(getEntityCollection());
    }

    public void testAddCallgroup() throws Exception {
        CallGroup cg = new CallGroup();
        cg.setUniqueId(1);
        cg.setName("sales");
        cg.setSipPassword("pass4321");

        m_credentials.generate(cg, m_credentials.findOrCreate(cg));

        String digest = Md5Encoder.digestPassword("sales", DOMAIN, "pass4321");
        assertObjectWithIdPresent("CallGroup1");
        assertObjectWithIdFieldValuePresent("CallGroup1", MongoConstants.IDENTITY, "sales@" + DOMAIN);
        assertObjectWithIdFieldValuePresent("CallGroup1", MongoConstants.PASSTOKEN, "pass4321");
        assertObjectWithIdFieldValuePresent("CallGroup1", MongoConstants.PINTOKEN, digest);
    }

    public void testAddUser() throws Exception {
        User user = new User();
        user.setUniqueId(1);
        user.setUserName("superadmin");
        final String PIN = "pin1234";
        user.setPin(PIN, DOMAIN);
        user.setSipPassword("pass4321");
        user.setDomainManager(getDomainManager());

        m_credentials.generate(user, m_credentials.findOrCreate(user));

        assertObjectWithIdPresent("User1");
        assertObjectWithIdFieldValuePresent("User1", MongoConstants.IDENTITY, "superadmin@" + DOMAIN);
        assertObjectWithIdFieldValuePresent("User1", MongoConstants.PASSTOKEN, "pass4321");
        assertObjectWithIdFieldValuePresent("User1", MongoConstants.PINTOKEN,
                Md5Encoder.digestPassword("superadmin", DOMAIN, PIN));
    }

    public void testAddUserEmptyPasswords() throws Exception {
        User user = new User();
        user.setUniqueId(1);
        user.setUserName("superadmin");
        user.setPin("", DOMAIN);
        user.setDomainManager(getDomainManager());

        m_credentials.generate(user, m_credentials.findOrCreate(user));

        assertObjectWithIdPresent("User1");
        assertObjectWithIdFieldValuePresent("User1", MongoConstants.IDENTITY, "superadmin@" + DOMAIN);
        String emptyHash = Md5Encoder.digestPassword("superadmin", DOMAIN, "");
        assertObjectWithIdFieldValuePresent("User1", MongoConstants.PASSTOKEN, "");
        assertObjectWithIdFieldValuePresent("User1", MongoConstants.PINTOKEN, emptyHash);
        assertObjectWithIdFieldValuePresent("User1", MongoConstants.REALM, DOMAIN);
    }

}
