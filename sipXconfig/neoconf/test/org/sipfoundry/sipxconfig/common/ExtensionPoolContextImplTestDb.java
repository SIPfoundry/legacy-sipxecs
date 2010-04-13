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
import org.springframework.context.ApplicationContext;

public class ExtensionPoolContextImplTestDb extends SipxDatabaseTestCase {
    private ExtensionPoolContext m_context;

    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_context = (ExtensionPoolContext) app.getBean(ExtensionPoolContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testDefaultUserExtensionPoolCreation() throws Exception {
        ExtensionPool pool = m_context.getUserExtensionPool();
        assertNotNull(pool);
    }

    public void testGetFreeUserExtension() throws Exception {
        TestHelper.cleanInsertFlat("common/TestUserExtensionPoolSeed.xml");
        ExtensionPool pool = m_context.getUserExtensionPool();

        // Find the first free extension
        assertEquals(5, m_context.getNextFreeUserExtension().intValue());

        // Push the next extension forward and search again
        pool.setNextExtension(11);
        m_context.saveExtensionPool(pool);
        assertEquals(17, m_context.getNextFreeUserExtension().intValue());

        // When we can't find an extension starting from the desired extension, we
        // should start again from the beginning of the range
        pool.setLastExtension(16);
        m_context.saveExtensionPool(pool);
        assertEquals(5, m_context.getNextFreeUserExtension().intValue());

        // When the pool is disabled, we should not get an extension
        pool.setEnabled(false);
        m_context.saveExtensionPool(pool);
        assertNull(m_context.getNextFreeUserExtension());
        pool.setEnabled(true);

        // When the first extension is null, we should not get an extension
        pool.setFirstExtension(null);
        m_context.saveExtensionPool(pool);
        assertNull(m_context.getNextFreeUserExtension());
        pool.setFirstExtension(1);

        // When the last extension is null, the search should be unbounded at the top
        final int BIG_FIRST_EXT = 999999;
        pool.setFirstExtension(BIG_FIRST_EXT);
        pool.setLastExtension(null);
        pool.setNextExtension(BIG_FIRST_EXT);
        m_context.saveExtensionPool(pool);
        assertEquals(BIG_FIRST_EXT, m_context.getNextFreeUserExtension().intValue());

        // When there are no free extensions, we should not get one
        pool.setFirstExtension(11);
        pool.setLastExtension(16);
        pool.setNextExtension(11);
        m_context.saveExtensionPool(pool);
        assertNull(m_context.getNextFreeUserExtension());
    }

}
