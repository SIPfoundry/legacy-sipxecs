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

import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;

public class IndexTrigger implements SetupListener {
    private IndexManager m_indexManager;
    private boolean m_enabled = true;
    private File m_indexDirectory;
    private boolean m_setupSecondPass;

    public void setIndexManager(IndexManager indexManager) {
        m_indexManager = indexManager;
    }

    public void setIndexDirectoryName(String indexDirectoryName) {
        m_indexDirectory = new File(indexDirectoryName);
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    @Override
    public boolean setup(SetupManager manager) {
        if (!m_enabled) {
            return true;
        }

        if (!m_indexDirectory.exists()) {

            // this ensures we index on 2nd pass after objects have had a chance to
            // migrate/update/whatever before we index them
            if (!m_setupSecondPass) {
                m_setupSecondPass = true;
                return false;
            }

            m_indexManager.indexAll();
        }
        return true;
    }
}
