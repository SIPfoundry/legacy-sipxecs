/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class DaoUtilsTestIntegration extends IntegrationTestCase {

    public void testCheckDuplicates() throws Exception {
        sql("common/UserSearchSeed.sql");

        User user = new User();
        user.setUserName("userseed1");
        assertTrue(DaoUtils.checkDuplicates(getHibernateTemplate(), User.class, user, "userName", null));

        user = new User();
        user.setUserName("wont find this guy");
        assertFalse(DaoUtils.checkDuplicates(getHibernateTemplate(), User.class, user, "userName", null));
    }
}
