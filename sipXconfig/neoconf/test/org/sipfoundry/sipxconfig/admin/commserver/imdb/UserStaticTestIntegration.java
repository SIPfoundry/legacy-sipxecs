/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import static org.sipfoundry.sipxconfig.admin.commserver.imdb.MongoTestCaseHelper.assertCollectionCount;
import static org.sipfoundry.sipxconfig.admin.commserver.imdb.MongoTestCaseHelper.assertObjectPresent;
import static org.sipfoundry.sipxconfig.admin.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValuePresent;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.common.User;

import com.mongodb.QueryBuilder;

public class UserStaticTestIntegration extends ImdbTestCase {
    private final String[][] USER_DATA = {
        {
            "0", "first1", "last1", "8809", "63948809"
        }, {
            "1", "first2", "last2", "8810", "63948810"
        }, {
            "2", "first3", "last3", "8811", "63948811"
        },
    };

    private List<User> m_users;
    private UserStatic m_userstaticDataSet;

    @Override
    public void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_users = new ArrayList<User>();
        for (String[] ud : USER_DATA) {
            User user = new User();
            user.setPermissionManager(getPermissionManager());

            user.setUniqueId(new Integer(ud[0]));
            user.setFirstName(ud[1]);
            user.setLastName(ud[2]);
            user.setUserName(ud[3]);
            user.setDomainManager(getDomainManager());
            user.setSettingValue("voicemail/mailbox/external-mwi", ud[4]);
            m_users.add(user);
        }
    }

    public void testGenerate() throws Exception {
        m_userstaticDataSet.generate(m_users.get(0), m_userstaticDataSet.findOrCreate(m_users.get(0)));
        m_userstaticDataSet.generate(m_users.get(1), m_userstaticDataSet.findOrCreate(m_users.get(1)));
        m_userstaticDataSet.generate(m_users.get(2), m_userstaticDataSet.findOrCreate(m_users.get(2)));
        assertCollectionCount(getEntityCollection(), 3);
        QueryBuilder qb = QueryBuilder.start(MongoConstants.ID);
        qb.is("User0").and(MongoConstants.STATIC+"."+MongoConstants.CONTACT).is("sip:"+USER_DATA[0][4]+"@"+DOMAIN);
        assertObjectPresent(getEntityCollection(), qb.get());
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User0", MongoConstants.STATIC+"."+MongoConstants.CONTACT, "sip:"+USER_DATA[0][4]+"@"+DOMAIN);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User0", MongoConstants.STATIC+"."+MongoConstants.TO_URI, "sip:"+USER_DATA[0][3]+"@"+DOMAIN);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.STATIC+"."+MongoConstants.EVENT, "message-summary");
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User2", MongoConstants.STATIC+"."+MongoConstants.FROM_URI, "sip:IVR@"+DOMAIN);
    }

    public void setUserstaticDataSet(UserStatic userstaticDataSet) {
        m_userstaticDataSet = userstaticDataSet;
    }
}
