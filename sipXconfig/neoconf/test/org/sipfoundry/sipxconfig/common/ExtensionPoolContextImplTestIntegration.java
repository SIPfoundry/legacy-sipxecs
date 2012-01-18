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

public class ExtensionPoolContextImplTestIntegration extends IntegrationTestCase {
    private ExtensionPoolContext m_extensionPoolContext;

    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testDefaultUserExtensionPoolCreation() throws Exception {
        ExtensionPool pool = m_extensionPoolContext.getUserExtensionPool();
        assertNotNull(pool);
    }

    public void testGetFreeUserExtension() throws Exception {
        loadDataSet("common/TestUserExtensionPoolSeed.xml");
        ExtensionPool pool = m_extensionPoolContext.getUserExtensionPool();

        // Find the first free extension
        assertEquals(5, m_extensionPoolContext.getNextFreeUserExtension().intValue());

        // Push the next extension forward and search again
        pool.setNextExtension(11);
        m_extensionPoolContext.saveExtensionPool(pool);
        assertEquals(17, m_extensionPoolContext.getNextFreeUserExtension().intValue());

        // When we can't find an extension starting from the desired extension, we
        // should start again from the beginning of the range
        pool.setLastExtension(16);
        m_extensionPoolContext.saveExtensionPool(pool);
        assertEquals(5, m_extensionPoolContext.getNextFreeUserExtension().intValue());

        // When the pool is disabled, we should not get an extension
        pool.setEnabled(false);
        m_extensionPoolContext.saveExtensionPool(pool);
        assertNull(m_extensionPoolContext.getNextFreeUserExtension());
        pool.setEnabled(true);

        // When the first extension is null, we should not get an extension
        pool.setFirstExtension(null);
        m_extensionPoolContext.saveExtensionPool(pool);
        assertNull(m_extensionPoolContext.getNextFreeUserExtension());
        pool.setFirstExtension(1);

        // When the last extension is null, the search should be unbounded at the top
        final int BIG_FIRST_EXT = 999999;
        pool.setFirstExtension(BIG_FIRST_EXT);
        pool.setLastExtension(null);
        pool.setNextExtension(BIG_FIRST_EXT);
        m_extensionPoolContext.saveExtensionPool(pool);
        assertEquals(BIG_FIRST_EXT, m_extensionPoolContext.getNextFreeUserExtension().intValue());

        // When there are no free extensions, we should not get one
        pool.setFirstExtension(11);
        pool.setLastExtension(16);
        pool.setNextExtension(11);
        m_extensionPoolContext.saveExtensionPool(pool);
        assertNull(m_extensionPoolContext.getNextFreeUserExtension());
    }

    public void setExtensionPoolContext(ExtensionPoolContext extensionPoolContext) {
        m_extensionPoolContext = extensionPoolContext;
    }

}
