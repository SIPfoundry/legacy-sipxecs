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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.setting.Group;

public class UserLocationTest extends MongoTestCase {

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

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        PermissionManagerImpl impl = new PermissionManagerImpl();
        impl.setModelFilesContext(TestHelper.getModelFilesContext());
        DomainManager dm = getDomainManager();
        replay(dm);
        m_users = new ArrayList<User>();
        for (String[] ud : USER_DATA) {
            User user = new User();
            user.setPermissionManager(impl);

            user.setUniqueId(new Integer(ud[0]));
            user.setFirstName(ud[1]);
            user.setLastName(ud[2]);
            user.setUserName(ud[3]);
            user.setDomainManager(dm);
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
    }

    public void testGenerateEmpty() throws Exception {
        UserLocation ul = new UserLocation();

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.emptyList());

        ul.setCoreContext(coreContext);

        replay(coreContext);

        ul.generate();
        assertCollectionCount(0);
    }

    public void testGenerate() throws Exception {
        UserLocation ul = new UserLocation();

        CoreContext coreContext = getCoreContext();
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(m_users);
        replay(coreContext);
        ul.setCoreContext(coreContext);
        ul.setDbCollection(getCollection());
        ul.generate();

        assertObjectWithIdFieldValuePresent("User0", UserLocation.LOCATION, USER_DATA[0][4]);
        assertObjectWithIdFieldValuePresent("User1", UserLocation.LOCATION, USER_DATA[1][4]);
        assertObjectWithIdFieldValuePresent("User2", UserLocation.LOCATION, USER_DATA[2][4]);
    }
}
