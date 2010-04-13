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

import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class IndexTrigger implements ApplicationListener {
    private IndexManager m_indexManager;

    private AdminContext m_adminContext;

    private boolean m_enabled = true;

    private File m_indexDirectory;

    public void onApplicationEvent(ApplicationEvent event) {
        if (!m_enabled) {
            return;
        }
        if (event instanceof ApplicationInitializedEvent) {
            if (!m_adminContext.inInitializationPhase()) {
                if (!m_indexDirectory.exists()) {
                    m_indexManager.indexAll();
                }
            }
        }
    }

    public void setIndexManager(IndexManager indexManager) {
        m_indexManager = indexManager;
    }

    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
    }

    public void setIndexDirectoryName(String indexDirectoryName) {
        m_indexDirectory = new File(indexDirectoryName);
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }
}
