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

import org.hibernate.SessionFactory;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.springframework.context.ApplicationContext;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class SipxHibernateDaoSupportTestDb extends SipxDatabaseTestCase {
    private static final String GROUP_NAME = "testCallGroup";
    private static final String COPY_OF = "CopyOf";
    private SipxHibernateDaoSupport m_dao;

    public static SipxHibernateDaoSupport createDao() {
        ApplicationContext app = TestHelper.getApplicationContext();
        SessionFactory sessionFactory = (SessionFactory) app.getBean("sessionFactory");
        SipxHibernateDaoSupport support = new SipxHibernateDaoSupport();
        support.setSessionFactory(sessionFactory);
        support.afterPropertiesSet();
        return support;
    }

    @Override
    protected void setUp() throws Exception {
        m_dao = createDao();
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testDuplicateBean() {
        CallGroup cg = new CallGroup();
        cg.setName(GROUP_NAME);
        HibernateTemplate hibernate = m_dao.getHibernateTemplate();
        hibernate.save(cg);

        // Make a copy and look for the copyOf prefix in the new bean's name
        CallGroup cg2 = (CallGroup) m_dao.duplicateBean(cg, "callGroupIdsWithName");
        assertEquals(COPY_OF + GROUP_NAME, cg2.getName());

        // Make another copy. Since we haven't saved cg2 yet, the name should
        // be the same as cg2.
        CallGroup cg3 = (CallGroup) m_dao.duplicateBean(cg, "callGroupIdsWithName");
        assertEquals(COPY_OF + GROUP_NAME, cg3.getName());

        // Make another copy after saving cg2.
        // This time the prefix should appear twice, so that
        // the name of the new bean will be unique.
        hibernate.save(cg2);
        CallGroup cg4 = (CallGroup) m_dao.duplicateBean(cg, "callGroupIdsWithName");
        assertEquals(COPY_OF + COPY_OF + GROUP_NAME, cg4.getName());

        // Clean up just to be nice. This isn't strictly necessary.
        hibernate.delete(cg);
        hibernate.delete(cg2);
        hibernate.flush();
    }

    public void testGetOriginalValue() throws Exception {
        TestHelper.cleanInsertFlat("common/TestUserSeed.db.xml");
        Integer id = new Integer(1000);
        User user = (User) m_dao.load(User.class, id);
        user.setUserName("goofy");
        assertEquals("testuser", m_dao.getOriginalValue(user, "userName"));
    }

    public void testGetOriginalValueIllegalProperty() throws Exception {
        TestHelper.cleanInsertFlat("common/TestUserSeed.db.xml");
        Integer id = new Integer(1000);
        User user = (User) m_dao.load(User.class, id);
        user.setUserName("goofy");
        try {
            m_dao.getOriginalValue(user, "eyeglassPerscription");
            fail();
        } catch (IllegalArgumentException e) {
            assertTrue(e.getMessage().indexOf("eyeglassPerscription") >= 0);
        }
    }
}
