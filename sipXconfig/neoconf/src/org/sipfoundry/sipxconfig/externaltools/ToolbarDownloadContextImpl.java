/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.externaltools;

import java.io.File;

import org.springframework.beans.factory.annotation.Required;

public class ToolbarDownloadContextImpl implements ToolbarDownloadContext {

    private String m_toolbarInstallerLocation;

    @Required
    public void setToolbarInstallerLocation(String toolbarInstallerLocation) {
        m_toolbarInstallerLocation = toolbarInstallerLocation;
    }

    @Override
    public File getToolbarInstaller() {
        return new File(m_toolbarInstallerLocation);
    }

    @Override
    public boolean isToolbarInstallerPresent() {
        return (new File(m_toolbarInstallerLocation)).canRead();
    }

}
