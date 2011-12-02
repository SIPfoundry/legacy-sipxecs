/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.monitoring;

import java.util.List;

public interface MonitoringContext {
    List<String> getAvailableHosts();

    List<String> getHosts();

    String getMrtgWorkingDir();

    List<MRTGTarget> getTargetsForHost(String host);

    List<MRTGTarget> getTargetsFromTemplate();

    List<String> getReports(String host);

    MRTGTarget getMRTGTarget(String reportName, String host);

    void generateConfigFiles(String host, List<String> reportNames);

    boolean updateGraphs(String host);

    boolean isEnabled();
}
