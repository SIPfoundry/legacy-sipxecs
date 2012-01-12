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

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;

import com.mongodb.DBObject;

public class UserLocationTestIntegration extends ImdbTestCase {

    private final String[][] USER_DATA = {
        {
            "0", "first1", "last1", "mir1", "boston"
        }, {
            "1", "first2", ",last2", "mir2", "seattle"
        }, {
            "2", "first3", ",last3", "mir3", null
        },
    };

    private List<User> m_users;
    private UserLocation m_userlocationDataSet;
    private ReplicationManagerImpl m_replManager;

    public void testGenerate() throws Exception {
        m_users = new ArrayList<User>();
        for (String[] ud : USER_DATA) {
            User user = new User();
            user.setPermissionManager(getPermissionManager());

            user.setUniqueId(new Integer(ud[0]));
            user.setFirstName(ud[1]);
            user.setLastName(ud[2]);
            user.setUserName(ud[3]);
            user.setDomainManager(getDomainManager());
            if (ud[4] != null) {
                Branch branch = new Branch();
                branch.setUniqueId();
                branch.setName(ud[4]);

                Group site = new Group();
                site.setUniqueId();
                site.setName("group" + ud[4]);
                site.setBranch(branch);

                user.addGroup(site);
            }
            m_users.add(user);
        }
        DBObject user1Obj = m_replManager.findOrCreate(m_users.get(0));
        m_userlocationDataSet.generate(m_users.get(0), user1Obj);
        assertEquals("User0", user1Obj.get(ID));
        assertEquals(USER_DATA[0][4], user1Obj.get(MongoConstants.USER_LOCATION));

        DBObject user2Obj = m_replManager.findOrCreate(m_users.get(1));
        m_userlocationDataSet.generate(m_users.get(1), user2Obj);
        assertEquals("User1", user2Obj.get(ID));
        assertEquals(USER_DATA[1][4], user2Obj.get(MongoConstants.USER_LOCATION));

        DBObject user3Obj = m_replManager.findOrCreate(m_users.get(2));
        m_userlocationDataSet.generate(m_users.get(2), user3Obj);
        assertEquals("User2", user3Obj.get(ID));
        assertEquals(USER_DATA[2][4], user3Obj.get(MongoConstants.USER_LOCATION));
    }

    public void setUserlocationDataSet(UserLocation userlocationDataSet) {
        m_userlocationDataSet = userlocationDataSet;
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replManager) {
        m_replManager = replManager;
    }
}
