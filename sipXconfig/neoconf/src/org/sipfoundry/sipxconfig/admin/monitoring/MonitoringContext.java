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
    public List<String> getAvailableHosts();

    public List<String> getHosts();

    public String getMrtgWorkingDir();

    public List<MRTGTarget> getTargetsForHost(String host);

    public List<MRTGTarget> getTargetsFromTemplate();

    public List<String> getReports(String host);

    public MRTGTarget getMRTGTarget(String reportName, String host);

    public void generateConfigFiles(String host, String communityString, List<String> reportNames);

    public boolean updateGraphs(String host);
}
