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

public interface ToolbarDownloadContext {

    boolean isToolbarInstallerPresent();

    File getToolbarInstaller();
}
