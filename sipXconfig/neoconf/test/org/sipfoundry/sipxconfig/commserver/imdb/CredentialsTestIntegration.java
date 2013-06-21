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

import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValueNotPresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValuePresent;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdPresent;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.commons.security.Md5Encoder;
import org.sipfoundry.sipxconfig.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.common.User;

public class CredentialsTestIntegration extends ImdbTestCase {

    public void testAddCallgroup() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        CallGroup cg = new CallGroup();
        cg.setUniqueId(1);
        cg.setName("sales");
        cg.setEnabled(true);
        cg.setSipPassword("pass4321");

        getReplicationManager().replicateEntity(cg, DataSet.CREDENTIAL);

        String digest = Md5Encoder.digestEncryptPassword("sales", DOMAIN, "pass4321");
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
        user.setPin(PIN);
        user.setSipPassword("pass4321");
        user.setDomainManager(getDomainManager());

        getReplicationManager().replicateEntity(user, DataSet.CREDENTIAL);

        assertObjectWithIdPresent(getEntityCollection(), "User1");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.IDENTITY, "superadmin@" + DOMAIN);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.PASSTOKEN, "pass4321");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.PINTOKEN,
                Md5Encoder.getEncodedPassword(PIN));
    }

    public void testAddUserEmptyPasswords() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        User user = new User();
        user.setUniqueId(1);
        user.setUserName("superadmin");
        user.setPin("");
        user.setDomainManager(getDomainManager());

        getReplicationManager().replicateEntity(user, DataSet.CREDENTIAL);

        assertObjectWithIdPresent(getEntityCollection(), "User1");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.IDENTITY, "superadmin@" + DOMAIN);
        String emptyHash = Md5Encoder.getEncodedPassword("");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.PASSTOKEN, "");
        assertObjectWithIdFieldValueNotPresent(getEntityCollection(), "User1", MongoConstants.PINTOKEN, emptyHash);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.REALM, DOMAIN);
    }

}
