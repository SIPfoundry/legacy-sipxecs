/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.common.User;

import com.mongodb.DBObject;

public class UserCallForwardTestIntegration extends ImdbTestCase {
    private final String[][] USER_DATA = {
        {
            "1", "first1", "last1", "mir1", "18"
        }, {
            "2", "first2", "last2", "mir2", "8"
        }, {
            "3", "first3", "last3", "mir3", "192"
        },
    };

    private List<User> m_users;
    private UserForward m_userforwardDataSet;
    private ReplicationManagerImpl m_replManager;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_users = new ArrayList<User>();
        for (String[] ud : USER_DATA) {
            User user = new User();
            user.setPermissionManager(getPermissionManager());
            user.setUniqueId(new Integer(ud[0]));
            user.setFirstName(ud[1]);
            user.setLastName(ud[2]);
            user.setUserName(ud[3]);
            user.setSettingTypedValue(CallSequence.CALL_FWD_TIMER_SETTING, ud[4]);
            user.setDomainManager(getDomainManager());
            m_users.add(user);
        }

        // add user with un-set value for cfwd timer - it is supposed to generate the default one
        // for this user at 20
        User user = new User();
        user.setPermissionManager(getPermissionManager());
        user.setDomainManager(getDomainManager());
        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("200");
        user.setUniqueId(4);
        m_users.add(user);
    }

    public void testGenerate() throws Exception {
        DBObject user1Obj = m_replManager.findOrCreate(m_users.get(0));
        m_userforwardDataSet.generate(m_users.get(0), user1Obj);
        assertEquals("User1", user1Obj.get(ID));
        assertEquals(Integer.valueOf(USER_DATA[0][4]), user1Obj.get(MongoConstants.CFWDTIME));

        DBObject user2Obj = m_replManager.findOrCreate(m_users.get(1));
        m_userforwardDataSet.generate(m_users.get(1), user2Obj);
        assertEquals("User2", user2Obj.get(ID));
        assertEquals(Integer.valueOf(USER_DATA[1][4]), user2Obj.get(MongoConstants.CFWDTIME));

        DBObject user3Obj = m_replManager.findOrCreate(m_users.get(2));
        m_userforwardDataSet.generate(m_users.get(2), user3Obj);
        assertEquals("User3", user3Obj.get(ID));
        assertEquals(Integer.valueOf(USER_DATA[2][4]), user3Obj.get(MongoConstants.CFWDTIME));

        DBObject user4Obj = m_replManager.findOrCreate(m_users.get(3));
        m_userforwardDataSet.generate(m_users.get(3), user4Obj);
        assertEquals("User4", user4Obj.get(ID));
        Integer defaultInitDelay = Integer.valueOf(getPermissionManager().getDefaultInitDelay());
        assertEquals(defaultInitDelay, user4Obj.get(MongoConstants.CFWDTIME));
    }

    public void setUserforwardDataSet(UserForward userforwardDataSet) {
        m_userforwardDataSet = userforwardDataSet;
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replManager) {
        m_replManager = replManager;
    }
}
