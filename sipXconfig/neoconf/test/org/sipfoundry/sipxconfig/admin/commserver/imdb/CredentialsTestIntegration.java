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

import com.mongodb.DBObject;

public class CredentialsTestIntegration extends ImdbTestCase {
    private Credentials m_credentialDataSet;
    private ReplicationManagerImpl m_replManager;

    public void testAddCallgroup() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        CallGroup cg = new CallGroup();
        cg.setUniqueId(1);
        cg.setName("sales");
        cg.setSipPassword("pass4321");

        DBObject cgObj = m_replManager.findOrCreate(cg);
        m_credentialDataSet.generate(cg, cgObj);
        String digest = Md5Encoder.digestPassword("sales", DOMAIN, "pass4321");
        assertEquals("sales@" + DOMAIN, cgObj.get(MongoConstants.IDENTITY));
        assertEquals("pass4321", cgObj.get(MongoConstants.PASSTOKEN));
        assertEquals(digest, cgObj.get(MongoConstants.PINTOKEN));
    }

    public void testAddUser() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        User user = new User();
        user.setUniqueId(1);
        user.setUserName("superadmin");
        final String PIN = "pin1234";
        user.setPin(PIN, DOMAIN);
        user.setSipPassword("pass4321");
        user.setDomainManager(getDomainManager());

        DBObject userObj = m_replManager.findOrCreate(user);
        m_credentialDataSet.generate(user, userObj);

        assertEquals("superadmin@" + DOMAIN, userObj.get(MongoConstants.IDENTITY));
        assertEquals("pass4321", userObj.get(MongoConstants.PASSTOKEN));
        assertEquals( Md5Encoder.digestPassword("superadmin", DOMAIN, PIN), userObj.get(MongoConstants.PINTOKEN));
    }

    public void testAddUserEmptyPasswords() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        User user = new User();
        user.setUniqueId(1);
        user.setUserName("superadmin");
        user.setPin("", DOMAIN);
        user.setDomainManager(getDomainManager());

        DBObject userObj = m_replManager.findOrCreate(user);
        m_credentialDataSet.generate(user, userObj);

        String emptyHash = Md5Encoder.digestPassword("superadmin", DOMAIN, "");
        assertEquals("", userObj.get(MongoConstants.PASSTOKEN));
        assertEquals(emptyHash, userObj.get(MongoConstants.PINTOKEN));
        assertEquals(DOMAIN, userObj.get(MongoConstants.REALM));
    }

    public void setCredentialDataSet(Credentials credentialDataSet) {
        m_credentialDataSet = credentialDataSet;
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replManager) {
        m_replManager = replManager;
    }

}
