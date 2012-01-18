/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValuePresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdPresent;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;

public class CredentialsTestIntegration extends ImdbTestCase {
    private Credentials m_credentialDataSet;

    public void testAddCallgroup() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        CallGroup cg = new CallGroup();
        cg.setUniqueId(1);
        cg.setName("sales");
        cg.setSipPassword("pass4321");

        m_credentialDataSet.generate(cg, m_credentialDataSet.findOrCreate(cg));

        String digest = Md5Encoder.digestPassword("sales", DOMAIN, "pass4321");
        assertObjectWithIdPresent(getEntityCollection(), "CallGroup1");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "CallGroup1", MongoConstants.IDENTITY, "sales@" + DOMAIN);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "CallGroup1", MongoConstants.PASSTOKEN, "pass4321");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "CallGroup1", MongoConstants.PINTOKEN, digest);
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

        m_credentialDataSet.generate(user, m_credentialDataSet.findOrCreate(user));

        assertObjectWithIdPresent(getEntityCollection(), "User1");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.IDENTITY, "superadmin@" + DOMAIN);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.PASSTOKEN, "pass4321");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.PINTOKEN,
                Md5Encoder.digestPassword("superadmin", DOMAIN, PIN));
    }

    public void testAddUserEmptyPasswords() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        User user = new User();
        user.setUniqueId(1);
        user.setUserName("superadmin");
        user.setPin("", DOMAIN);
        user.setDomainManager(getDomainManager());

        m_credentialDataSet.generate(user, m_credentialDataSet.findOrCreate(user));

        assertObjectWithIdPresent(getEntityCollection(), "User1");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.IDENTITY, "superadmin@" + DOMAIN);
        String emptyHash = Md5Encoder.digestPassword("superadmin", DOMAIN, "");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.PASSTOKEN, "");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.PINTOKEN, emptyHash);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.REALM, DOMAIN);
    }

    public void setCredentialDataSet(Credentials credentialDataSet) {
        m_credentialDataSet = credentialDataSet;
    }

}
