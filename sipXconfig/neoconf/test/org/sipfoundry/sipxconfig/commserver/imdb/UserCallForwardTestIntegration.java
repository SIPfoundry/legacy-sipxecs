/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertCollectionCount;
import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValuePresent;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.time.NtpManager;

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
    private ProxyManager m_proxyManager;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_users = new ArrayList<User>();
        for (String[] ud : USER_DATA) {
            User user = getCoreContext().newUser();
            user.setUniqueId(new Integer(ud[0]));
            user.setFirstName(ud[1]);
            user.setLastName(ud[2]);
            user.setUserName(ud[3]);
            user.setSettingTypedValue(CallSequence.CALL_FWD_TIMER_SETTING, ud[4]);
            m_users.add(user);
        }

        // add user with un-set value for cfwd timer - it is supposed to generate the default one
        // for this user at 20
        User user = getCoreContext().newUser();
        
        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("200");
        user.setUniqueId(4);
        m_users.add(user);
    }

    public void testGenerate() throws Exception {
        getReplicationManager().replicateEntity(m_users.get(0), DataSet.USER_FORWARD);
        getReplicationManager().replicateEntity(m_users.get(1), DataSet.USER_FORWARD);
        getReplicationManager().replicateEntity(m_users.get(2), DataSet.USER_FORWARD);
        getReplicationManager().replicateEntity(m_users.get(3), DataSet.USER_FORWARD);
        assertCollectionCount(getEntityCollection(), 4);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User1", MongoConstants.CFWDTIME, Integer.valueOf(USER_DATA[0][4]));
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User2", MongoConstants.CFWDTIME, Integer.valueOf(USER_DATA[1][4]));
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User3", MongoConstants.CFWDTIME, Integer.valueOf(USER_DATA[2][4]));
        Integer defaultInitDelay = Integer.valueOf(m_proxyManager.getSettings().getDefaultInitDelay());
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "User4", MongoConstants.CFWDTIME, defaultInitDelay);
    }

    public void setProxyManager(ProxyManager proxyManager) {
        m_proxyManager = proxyManager;
    }

}
