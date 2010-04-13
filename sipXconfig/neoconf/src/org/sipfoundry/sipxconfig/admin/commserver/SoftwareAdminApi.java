/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.List;


public interface SoftwareAdminApi {
    List<String> exec(String hostName, String command);
    List<String> snapshot(String hostname, String[] commandArguments);
    String execStatus(String hostName, String command);
}
