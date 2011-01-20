/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public class UserCallForwardTest extends MongoTestCase {

    private final String[][] USER_DATA = {
        {
            "1", "first1", "last1", "mir1", "20"
        }, {
            "2", "first2", "last2", "mir2", "8"
        }, {
            "3", "first3", "last3", "mir3", "192"
        },
    };

    private List<User> m_users;
    private List<CallSequence> m_cs;
    private PermissionManager m_permManager;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        ModelFilesContext mfc = TestHelper.getModelFilesContext();
        Setting settings1 = mfc.loadModelFile("commserver/user-settings.xml");
        Setting settings2 = mfc.loadModelFile("commserver/user-settings.xml");
        Setting settings3 = mfc.loadModelFile("commserver/user-settings.xml");
        Setting settings4 = mfc.loadModelFile("commserver/user-settings.xml");

        m_permManager = createMock(PermissionManager.class);
        m_permManager.getDefaultInitDelay();
        expectLastCall().andReturn("54").anyTimes();

        m_permManager.getPermissionModel();
        expectLastCall().andReturn(settings1).once();
        m_permManager.getPermissionModel();
        expectLastCall().andReturn(settings2).once();
        m_permManager.getPermissionModel();
        expectLastCall().andReturn(settings3).once();
        m_permManager.getPermissionModel();
        expectLastCall().andReturn(settings4).once();

        m_permManager.setModelFilesContext(mfc);
        expectLastCall().anyTimes();

        DomainManager dm = getDomainManager();

        replay(m_permManager, dm);

        m_users = new ArrayList<User>();
        m_cs = new ArrayList<CallSequence>();
        for (String[] ud : USER_DATA) {
            User user = new User();
            user.setPermissionManager(m_permManager);
            user.setUniqueId(new Integer(ud[0]));
            user.setFirstName(ud[1]);
            user.setLastName(ud[2]);
            user.setUserName(ud[3]);
            user.setDomainManager(dm);
            m_users.add(user);

            CallSequence cs = new CallSequence();
            cs.setUser(user);
            cs.setCfwdTime(Integer.valueOf(ud[4]));

            m_cs.add(cs);
        }
        // add user with un-set value for cfwd timer - it is supposed to generate the default one
        // for this user(54)
        User user = createDefaultCfwdUser();
        user.setDomainManager(dm);
        user.setUniqueId(4);
        m_users.add(user);
        CallSequence cs = new CallSequence();
        cs.setUser(user);
        m_cs.add(cs);

    }

    private User createDefaultCfwdUser() {
        User user = new User();
        user.setPermissionManager(m_permManager);

        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("200");

        return user;
    }

    public void testGenerateEmpty() throws Exception {
        UserForward uf = new UserForward();
        CoreContext coreContext = getCoreContext();
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.EMPTY_LIST).anyTimes();
        replay(coreContext);
        uf.setCoreContext(getCoreContext());
        uf.generate();
        uf.setDbCollection(getCollection());
        assertCollectionCount(0);
    }

    public void testGenerate() throws Exception {
        UserForward uf = new UserForward();

        CoreContext coreContext = getCoreContext();
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(m_users);
        ForwardingContext forwardingContext = createMock(ForwardingContext.class);
        for (int i = 0; i < m_users.size(); i++) {
            forwardingContext.getCallSequenceForUser(m_users.get(i));
            expectLastCall().andReturn(m_cs.get(i));
        }

        uf.setCoreContext(coreContext);
        uf.setForwardingContext(forwardingContext);
        uf.setDbCollection(getCollection());
        replay(coreContext, forwardingContext);
        uf.generate();
        assertCollectionCount(4);

        assertObjectWithIdFieldValuePresent("User1", UserForward.CFWDTIME, USER_DATA[0][4]);
        assertObjectWithIdFieldValuePresent("User2", UserForward.CFWDTIME, USER_DATA[1][4]);
        assertObjectWithIdFieldValuePresent("User3", UserForward.CFWDTIME, USER_DATA[2][4]);
        assertObjectWithIdFieldValuePresent("User4", UserForward.CFWDTIME, "54");
    }
}
