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

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.StringTokenizer;

public class MonitoringUtil {
    public static final String MRTG_DIR = "/usr/bin/mrtg";

    public static final String DAY_SUMMARY_EXTENSION = "-day-s";
    public static final String DAY_EXTENSION = "-day";
    public static final String WEEK_EXTENSION = "-week";
    public static final String MONTH_EXTENSION = "-month";
    public static final String YEAR_EXTENSION = "-year";
    public static final String GRAPH_EXTENSION = ".png";
    public static final String NUMERIC_STATS_EXTENSION = ".sts";

    public static final String COLON = ":";
    public static final String UNDERSCORE = "_";
    public static final String SPACE = " ";
    public static final String FORWARD_SLASH = "/";
    public static final String NEW_LINE = "\n";
    public static final String NBSP = "&nbsp;";

    private static final String HTML_FREE = "<font color='#008000'>Free</font>";
    private static final String HTML_TOTAL = "<font color='#0000FF'>Total</font>";

    protected MonitoringUtil() {
        throw new UnsupportedOperationException();
    }

    /**
     * creates the html statistics that will be displayed under the image
     * for the given report
     * Highlights 'Free' in green and 'Total' with blue
     */
    public static String getHtmlDetailsForGraph(MRTGTarget target, String report,
            String workingDir) {
        try {
            String path = workingDir + FORWARD_SLASH + target.getId() + report
                    + NUMERIC_STATS_EXTENSION;
            BufferedReader reader = new BufferedReader(new FileReader(path));
            StringBuilder details = new StringBuilder();
            for (String line = reader.readLine(); line != null; line = reader.readLine()) {
                StringTokenizer st = new StringTokenizer(line, "\"", false);
                while (st.hasMoreTokens()) {
                    String detail = st.nextToken();
                    details.append(NBSP);
                    detail = detail.replace("Free", HTML_FREE).replace("Total", HTML_TOTAL);
                    details.append(detail);
                    details.append(NBSP);
                    details.append(target.getShortLegend());
                    if (line.contains("Current")) {
                        details.append("<br/>");
                    }
                }
            }
            reader.close();
            return details.toString();
        } catch (IOException ioEx) {
            // no statistics to display
            return "";
        }
    }
}
