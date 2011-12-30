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

import org.sipfoundry.sipxconfig.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class SipxHibernateDaoSupportTestIntegration extends IntegrationTestCase {
    private static final String GROUP_NAME = "testCallGroup";
    private static final String COPY_OF = "CopyOf";
    private SipxHibernateDaoSupport<User> m_user;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();        
        m_user = new SipxHibernateDaoSupport<User>();
        m_user.setSessionFactory(getSessionFactory());
        m_user.afterPropertiesSet();
        clear();
    }

    public void testDuplicateBean() {
        SipxHibernateDaoSupport<CallGroup> dao = new SipxHibernateDaoSupport<CallGroup>();
        dao.setSessionFactory(getSessionFactory());
        dao.afterPropertiesSet();

        CallGroup cg = new CallGroup();
        cg.setName(GROUP_NAME);
        HibernateTemplate hibernate = getHibernateTemplate();
        hibernate.save(cg);

        // Make a copy and look for the copyOf prefix in the new bean's name
        CallGroup cg2 = (CallGroup) dao.duplicateBean(cg, "callGroupIdsWithName");
        assertEquals(COPY_OF + GROUP_NAME, cg2.getName());

        // Make another copy. Since we haven't saved cg2 yet, the name should
        // be the same as cg2.
        CallGroup cg3 = (CallGroup) dao.duplicateBean(cg, "callGroupIdsWithName");
        assertEquals(COPY_OF + GROUP_NAME, cg3.getName());

        // Make another copy after saving cg2.
        // This time the prefix should appear twice, so that
        // the name of the new bean will be unique.
        hibernate.save(cg2);
        CallGroup cg4 = (CallGroup) dao.duplicateBean(cg, "callGroupIdsWithName");
        assertEquals(COPY_OF + COPY_OF + GROUP_NAME, cg4.getName());

        // Clean up just to be nice. This isn't strictly necessary.
        hibernate.delete(cg);
        hibernate.delete(cg2);
        hibernate.flush();
    }

    public void testGetOriginalValue() throws Exception {
        sql("common/TestUserSeed.sql");
        Integer id = new Integer(1000);
        User user = (User) m_user.load(User.class, id);
        user.setUserName("goofy");
        assertEquals("testuser", m_user.getOriginalValue(user, "userName"));
    }

    public void testGetOriginalValueIllegalProperty() throws Exception {
        sql("common/TestUserSeed.sql");
        Integer id = new Integer(1000);
        User user = (User) m_user.load(User.class, id);
        user.setUserName("goofy");
        try {
            m_user.getOriginalValue(user, "eyeglassPerscription");
            fail();
        } catch (IllegalArgumentException e) {
            assertTrue(e.getMessage().indexOf("eyeglassPerscription") >= 0);
        }
    }
}
