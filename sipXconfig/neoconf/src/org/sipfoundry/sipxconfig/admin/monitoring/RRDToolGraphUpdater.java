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
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.StringTokenizer;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class RRDToolGraphUpdater {
    private static final Log LOG = LogFactory.getLog(RRDToolGraphUpdater.class);
    private static final String BACKGROUND_COLOR = "f5f5f5";
    private static final String LINE_COLOR = "0000ff";
    private static final String AREA_COLOR = "00cc00";
    private static final String COLOR = "000000";
    private static final String RRD_TOOL_COMMAND = "rrdtool";
    private static final String RRD_GRAPH_COMMAND = "graph";
    private static final String RRD_FILE_EXTENSION = ".rrd";

    private static final String BACK_SLASH = "\\";
    private static final String DASH = "-";
    private static final String C_LETTER = "c";
    private static final String H_LETTER = "h";
    private static final String W_LETTER = "w";

    private static final String AREA_IN = "AREA:in#";
    private static final String DEF_IN = "DEF:in=";
    private static final String DEF_LINE2 = "DEF:line2=";
    private static final String DS0_AVERAGE = ":ds0:AVERAGE";
    private static final String DS1_AVERAGE = ":ds1:AVERAGE";
    private static final String LINE2 = "LINE2:line2#";
    private static final String PRINT_AVERAGE = "PRINT:in:AVERAGE:\"Average ";
    private static final String PRINT_AVERAGE_LINE2 = "PRINT:line2:AVERAGE:\"Average ";
    private static final String PRINT_MAX = "PRINT:in:MAX:\"Max ";
    private static final String PRINT_MAX_LINE2 = "PRINT:line2:MAX:\"Max ";
    private static final String PRINT_LAST = "PRINT:in:LAST:\"Current ";
    private static final String PRINT_LAST_LINE2 = "PRINT:line2:LAST:\"Current ";
    private static final String LF = "=%.1lf %s\"";

    private static final String FILE_SEPARATOR = "file.separator";
    private static final String TOOL_ERROR = "\nAn error occurred while running graphing tool.";

    private static final long SECONDS_PER_DAY = 24 * 60 * 60;
    private static final long SECONDS_PER_WEEK = 7 * 24 * 60 * 60;
    private static final long SECONDS_PER_MONTH = 30 * 24 * 60 * 60;
    private static final long SECONDS_PER_YEAR = 365 * 24 * 60 * 60;

    private static final int DETAILED_GRAPH_X_SIZE = 400;
    private static final int DETAILED_GRAPH_Y_SIZE = 150;
    private static final int SUMMARY_GRAPH_X_SIZE = 200;
    private static final int SUMMARY_GRAPH_Y_SIZE = 100;

    private String m_lastErrors = StringUtils.EMPTY;
    private String m_dataDirectory = StringUtils.EMPTY;
    private String m_graphDirectory = StringUtils.EMPTY;

    private int m_successExitCode;

    public RRDToolGraphUpdater(String directory) {
        this(directory, directory);
    }

    public RRDToolGraphUpdater(String dataDirectory, String graphDirectory) {
        m_dataDirectory = dataDirectory;
        m_graphDirectory = graphDirectory;

        if (!m_dataDirectory.endsWith(MonitoringUtil.FORWARD_SLASH)
                && !m_dataDirectory.endsWith(BACK_SLASH)) {
            m_dataDirectory += System.getProperty(FILE_SEPARATOR);
        }
        if (!m_graphDirectory.endsWith(MonitoringUtil.FORWARD_SLASH)
                && !m_graphDirectory.endsWith(BACK_SLASH)) {
            m_graphDirectory += System.getProperty(FILE_SEPARATOR);
        }
    }

    private boolean graphNeedsUpdate(MRTGTarget target, String graphNameExt) throws IOException {
        boolean result = false;
        String rrdFileName = m_dataDirectory + target.getId() + RRD_FILE_EXTENSION;
        String rrdGraphName = m_graphDirectory + target.getId() + graphNameExt;

        File rrdFile = new File(rrdFileName);
        if (!rrdFile.exists()) {
            throw new IOException("Data file does not exist: " + rrdFileName);
        }
        File rrdGraphFile = new File(rrdGraphName);
        if (!rrdGraphFile.exists() || rrdGraphFile.lastModified() < rrdFile.lastModified()) {
            result = true;
        }
        return result;
    }

    private void writePrintInfo(MRTGTarget target, String intervalExt,
                                List<String> lines) throws IOException {
        String eol = System.getProperty("line.separator");
        String name = m_graphDirectory + target.getId() + intervalExt
                + MonitoringUtil.NUMERIC_STATS_EXTENSION;
        BufferedWriter out = new BufferedWriter(new FileWriter(name));
        for (int i = 0; i < lines.size(); i++) {
            out.write(lines.get(i) + eol);
        }
        out.close();
    }

    private void executeCommand(MRTGTarget target, String intervalExt,
                                String[] command) throws Exception {
        int exitValue = 0;
        List<String> results = new ArrayList<String>();
        Runtime rt = Runtime.getRuntime();
        Process p = rt.exec(command);

        StreamGobbler outGobbler = new StreamGobbler(p.getInputStream());
        StreamGobbler errGobbler = new StreamGobbler(p.getErrorStream());
        outGobbler.start();
        errGobbler.start();
        try {
            exitValue = p.waitFor();
        } catch (InterruptedException e) {
            exitValue = m_successExitCode + 100;
            m_lastErrors += TOOL_ERROR;
            m_lastErrors += MonitoringUtil.NEW_LINE + e.getMessage();
        } catch (Throwable e) {
            exitValue = m_successExitCode + 200;
            m_lastErrors += TOOL_ERROR;
            m_lastErrors += e.getMessage();
        }

        List<String> outlines = outGobbler.getOutputLines();
        for (int i = 0; i < outlines.size(); i++) {
            StringTokenizer st = new StringTokenizer(outlines.get(i), "=");
            if (st.hasMoreTokens()) {
                st.nextToken();
                if (st.hasMoreTokens()) {
                    // this is line we expect, save result
                    results.add(outlines.get(i));
                }
            }
        }

        List<String> errlines = errGobbler.getOutputLines();

        for (int i = 0; i < errlines.size(); i++) {
            m_lastErrors += MonitoringUtil.NEW_LINE + errlines.get(i);
        }

        if (results.size() > 0) {
            writePrintInfo(target, intervalExt, results);
        }

        if (exitValue != m_successExitCode) {
            throw new GraphCreationException(m_lastErrors);
        }
    }

    private void updateGraph(MRTGTarget target, boolean detailed, long duration,
            String intervalExt) throws Exception {
        String graphExt = intervalExt + MonitoringUtil.GRAPH_EXTENSION;
        if (!graphNeedsUpdate(target, graphExt)) {
            return;
        }
        String dataFileName = m_dataDirectory + target.getId() + RRD_FILE_EXTENSION;
        // create the graph
        GregorianCalendar c = new GregorianCalendar();
        long currentTimeSeconds = c.getTime().getTime() / 1000;

        List<String> commandVector = new ArrayList<String>();
        commandVector.add(RRD_TOOL_COMMAND);
        commandVector.add(RRD_GRAPH_COMMAND);
        commandVector.add(m_graphDirectory + target.getId() + graphExt);
        commandVector.add("--start");
        commandVector.add(new Long(currentTimeSeconds - duration) + StringUtils.EMPTY);
        commandVector.add("--end");
        commandVector.add(currentTimeSeconds + StringUtils.EMPTY);
        commandVector.add(DASH + C_LETTER);
        commandVector.add("FONT#" + COLOR);
        commandVector.add(DASH + C_LETTER);
        commandVector.add("MGRID#" + COLOR);
        commandVector.add(DASH + C_LETTER);
        commandVector.add("FRAME#" + COLOR);
        commandVector.add(DASH + C_LETTER);
        commandVector.add("BACK#" + BACKGROUND_COLOR);
        commandVector.add(DASH + C_LETTER);
        commandVector.add("ARROW#" + COLOR);

        if (target.getMaxBytes() < Integer.MAX_VALUE) {
            commandVector.add("-u");
            commandVector.add(new Long(target.getMaxBytes()).toString());
        }
        commandVector.add("-l");
        commandVector.add("0");
        commandVector.add("-v");
        commandVector.add(target.getYLegend());

        if (detailed) {
            commandVector.add(DASH + W_LETTER);
            commandVector.add(StringUtils.EMPTY + DETAILED_GRAPH_X_SIZE);
            commandVector.add(DASH + H_LETTER);
            commandVector.add(StringUtils.EMPTY + DETAILED_GRAPH_Y_SIZE);
            if (StringUtils.isNotEmpty(target.getLegend1())) {
                commandVector.add(DEF_IN + dataFileName + DS0_AVERAGE);
                commandVector.add(AREA_IN + AREA_COLOR + MonitoringUtil.COLON
                        + target.getLegend1());
                commandVector.add(PRINT_AVERAGE + target.getLegendI() + LF);
                commandVector.add(PRINT_MAX + target.getLegendI() + LF);
                commandVector.add(PRINT_LAST + target.getLegendI() + LF);
            }
            if (StringUtils.isNotEmpty(target.getLegend2())) {
                commandVector.add(DEF_LINE2 + dataFileName + DS1_AVERAGE);
                commandVector.add(LINE2 + LINE_COLOR + MonitoringUtil.COLON
                        + target.getLegend2());
                commandVector.add(PRINT_AVERAGE_LINE2 + target.getLegendO() + LF);
                commandVector.add(PRINT_MAX_LINE2 + target.getLegendO() + LF);
                commandVector.add(PRINT_LAST_LINE2 + target.getLegendO() + LF);
            }
        } else {
            commandVector.add(DASH + W_LETTER);
            commandVector.add(StringUtils.EMPTY + SUMMARY_GRAPH_X_SIZE);
            commandVector.add(DASH + H_LETTER);
            commandVector.add(StringUtils.EMPTY + SUMMARY_GRAPH_Y_SIZE);
            if (StringUtils.isNotEmpty(target.getLegendI())) {
                commandVector.add(DEF_IN + dataFileName + DS0_AVERAGE);
                commandVector.add(AREA_IN + AREA_COLOR + MonitoringUtil.COLON
                        + target.getLegendI());
            }
            if (StringUtils.isNotEmpty(target.getLegendO())) {
                commandVector.add(DEF_LINE2 + dataFileName + DS1_AVERAGE);
                commandVector.add(LINE2 + LINE_COLOR + MonitoringUtil.COLON
                        + target.getLegendO());
            }
        }
        String[] commandArray = new String[commandVector.size()];
        for (int i = 0; i < commandVector.size(); i++) {
            commandArray[i] = commandVector.get(i);
        }
        executeCommand(target, intervalExt, commandArray);
    }

    public void updateSummaryGraph(MRTGTarget target) throws Exception {
        updateGraph(target, false, SECONDS_PER_DAY, MonitoringUtil.DAY_SUMMARY_EXTENSION);
    }

    public void updateDailyGraph(MRTGTarget target) throws Exception {
        updateGraph(target, true, SECONDS_PER_DAY, MonitoringUtil.DAY_EXTENSION);
    }

    public void updateWeeklyGraph(MRTGTarget target) throws Exception {
        updateGraph(target, true, SECONDS_PER_WEEK, MonitoringUtil.WEEK_EXTENSION);
    }

    public void updateMonthlyGraph(MRTGTarget target) throws Exception {
        updateGraph(target, true, SECONDS_PER_MONTH, MonitoringUtil.MONTH_EXTENSION);
    }

    public void updateYearlyGraph(MRTGTarget target) throws Exception {
        updateGraph(target, true, SECONDS_PER_YEAR, MonitoringUtil.YEAR_EXTENSION);
    }

    public void updateAllGraphs(MRTGTarget target) throws Exception {
        updateSummaryGraph(target);
        updateDailyGraph(target);
        updateWeeklyGraph(target);
        updateMonthlyGraph(target);
        updateYearlyGraph(target);
    }

    private class StreamGobbler extends Thread {
        private InputStream m_in;
        private List<String> m_lines = new ArrayList<String>();

        public StreamGobbler(InputStream is) {
            m_in = is;
        }

        public List<String> getOutputLines() {
            return m_lines;
        }

        public void run() {
            try {
                BufferedReader br = new BufferedReader(new InputStreamReader(m_in));
                String line = null;
                while ((line = br.readLine()) != null) {
                    m_lines.add(line);
                }
            } catch (IOException e) {
                LOG.error(e);
            }
        }

    }

    private class GraphCreationException extends Exception {

        public GraphCreationException(String message) {
            super(message);
        }
    }
}
