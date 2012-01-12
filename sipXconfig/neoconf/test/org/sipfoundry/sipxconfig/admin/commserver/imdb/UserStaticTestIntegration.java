/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.common.User;

import com.mongodb.DBObject;

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
    private ReplicationManagerImpl m_replManager;

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
        DBObject user1Obj = m_replManager.findOrCreate(m_users.get(0));
        m_userstaticDataSet.generate(m_users.get(0), user1Obj);
        assertEquals("User0", user1Obj.get(ID));
        DBObject staticObj = (DBObject) user1Obj.get(MongoConstants.STATIC);
        assertEquals("sip:"+USER_DATA[0][4]+"@"+DOMAIN, staticObj.get(MongoConstants.CONTACT));
        assertEquals("sip:"+USER_DATA[0][3]+"@"+DOMAIN, staticObj.get(MongoConstants.TO_URI));

        DBObject user2Obj = m_replManager.findOrCreate(m_users.get(1));
        m_userstaticDataSet.generate(m_users.get(1), user2Obj);
        assertEquals("User1", user2Obj.get(ID));
        staticObj = (DBObject) user2Obj.get(MongoConstants.STATIC);
        assertEquals("message-summary", staticObj.get(MongoConstants.EVENT));

        DBObject user3Obj = m_replManager.findOrCreate(m_users.get(2));
        m_userstaticDataSet.generate(m_users.get(2), user3Obj);
        assertEquals("User2", user3Obj.get(ID));
        staticObj = (DBObject) user3Obj.get(MongoConstants.STATIC);
        assertEquals("sip:IVR@"+DOMAIN, staticObj.get(MongoConstants.FROM_URI));
    }

    public void setUserstaticDataSet(UserStatic userstaticDataSet) {
        m_userstaticDataSet = userstaticDataSet;
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replManager) {
        m_replManager = replManager;
    }
}
