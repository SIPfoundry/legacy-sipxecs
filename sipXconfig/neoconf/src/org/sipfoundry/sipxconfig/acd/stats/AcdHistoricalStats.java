/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats;

import java.io.IOException;
import java.io.Writer;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public interface AcdHistoricalStats {

    public static final String BEAN_NAME = "acdHistoricalStats";

    public List<String> getReports();

    public List<String> getReportFields(String reportName);

    public List<Map<String, Object>> getReport(String name, Date startTime, Date endTime, Location location);

    public void dumpReport(Writer writer, List<Map<String, Object>> reportData, Locale locale) throws IOException;

    public boolean isEnabled();
}
