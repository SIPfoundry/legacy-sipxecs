/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.update;

import java.util.List;

public interface UpdateApi {
    // This string must match the output of the sipx-sqadmin.py script when sipxecs rpms are not installed
    static final String VERSION_NOT_DETERMINED = "Could not determine version";
    void installUpdates();
    String getCurrentVersion();
    List<PackageUpdate> getAvailableUpdates();
}
