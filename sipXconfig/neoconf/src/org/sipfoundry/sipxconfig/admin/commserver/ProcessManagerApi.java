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
import java.util.Map;

public interface ProcessManagerApi {
    Map<String, String> getStateAll(String host);

    List<String> getStatusMessages(String host, String process);

    Map<String, String> stop(String host, String[] processes, boolean block);

    Map<String, String> start(String host, String[] processes, boolean block);

    Map<String, String> restart(String host, String[] processes, boolean block);

    boolean setConfigVersion(String host, String service, String version);
}
