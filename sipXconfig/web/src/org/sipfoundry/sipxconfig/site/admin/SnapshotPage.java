/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.File;

import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.Snapshot;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class SnapshotPage extends BasePage {

    public abstract void setSnapshotFile(File file);

    public abstract Snapshot getSnapshot();

    public void createSnapshot() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        File file = getSnapshot().perform();
        setSnapshotFile(file);
    }
}
