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

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class DaoUtilsTestDb extends SipxDatabaseTestCase {
    private SipxHibernateDaoSupport m_dao;
    private HibernateTemplate m_hibernate;

    protected void setUp() throws Exception {
        m_dao = SipxHibernateDaoSupportTestDb.createDao();
        m_hibernate = m_dao.getHibernateTemplate();
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testCheckDuplicates() throws Exception {
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");

        User user = new User();
        user.setUserName("userseed1");
        assertTrue(DaoUtils.checkDuplicates(m_hibernate, User.class, user, "userName", null));

        user = new User();
        user.setUserName("wont find this guy");
        assertFalse(DaoUtils.checkDuplicates(m_hibernate, User.class, user, "userName", null));
    }
}
