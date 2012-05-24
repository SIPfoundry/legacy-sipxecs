/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.search;

import java.io.File;

import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class IndexTrigger implements ApplicationListener<ApplicationEvent> {
    private IndexManager m_indexManager;
    private SetupManager m_setupManager;
    private boolean m_enabled = true;
    private File m_indexDirectory;

    @Override
    public void onApplicationEvent(ApplicationEvent event) {
        if (!m_enabled) {
            return;
        }
        if (event instanceof ApplicationInitializedEvent) {
            // calling SetupManager.setup() forces SetupManager to run first as order is not
            // guaranteed. Calling multiple times is harmless (idempotent)
            m_setupManager.setup();
            if (!m_indexDirectory.exists()) {
                m_indexManager.indexAll();
            }
        }
    }

    public void setIndexManager(IndexManager indexManager) {
        m_indexManager = indexManager;
    }

    public void setIndexDirectoryName(String indexDirectoryName) {
        m_indexDirectory = new File(indexDirectoryName);
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public void setSetupManager(SetupManager setupManager) {
        m_setupManager = setupManager;
    }
}
