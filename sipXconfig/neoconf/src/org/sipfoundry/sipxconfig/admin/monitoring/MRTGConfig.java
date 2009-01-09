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
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;

public class MRTGConfig {
    private static final String RUN_AS_DAEMON_TOKEN = "RunAsDaemon";
    private static final String NO_DETACH_TOKEN = "NoDetach";
    private static final String INTERVAL_TOKEN = "Interval";
    private static final String WORK_DIR_TOKEN = "workdir";
    private static final String THRESH_DIR_TOKEN = "threshdir";
    private static final String TARGET_TOKEN = "target";
    private static final String TITLE_TOKEN = "title";
    private static final String PAGE_TOP_TOKEN = "pagetop";
    private static final String MAX_BYTES_TOKEN = "maxbytes";
    private static final String SHORT_LEGEND_TOKEN = "shortlegend";
    private static final String Y_LEGEND_TOKEN = "ylegend";
    private static final String LEGEND_I_TOKEN = "legendi";
    private static final String LEGEND_O_TOKEN = "legendo";
    private static final String LEGEND_1_TOKEN = "legend1";
    private static final String LEGEND_2_TOKEN = "legend2";
    private static final String OPTIONS_TOKEN = "options";
    private static final String UNSCALED_TOKEN = "unscaled";
    private static final String KMG_TOKEN = "kmg";
    private static final String FACTOR_TOKEN = "factor";
    private static final String KILO_TOKEN = "kilo";
    private static final String LOADMIBS_TOKEN = "LoadMibs";
    private static final String IPV6_TOKEN = "EnableIPv6";
    private static final String LOG_FORMAT_TOKEN = "LogFormat";
    private static final String PATH_ADD_TOKEN = "PathAdd";
    private static final String THRESH_MAX_TOKEN = "threshMaxO";
    private static final String THRESH_PROG_TOKEN = "threshProgO";
    private static final String THRESH_PROG_OK_TOKEN = "threshProgOKO";
    private static final String THRESH_DESC_TOKEN = "threshDesc";
    private static final String DEFAULT_LOG_FORMAT = "rrdtool";
    private static final String DEFAULT_PATH_ADD = "/usr/bin";

    private static final String COMMA = ",";
    private static final String POUND = "#";
    private static final String OPEN_BRACKET = "[";
    private static final String CLOSE_BRACKET = "]";
    private static final String EOL = System.getProperty("line.separator");
    private static final String YES = "Yes";

    private String m_workingDir;
    private String m_threshDir;
    private String m_runAsDaemon = YES;
    private String m_noDetach = YES;
    private String m_interval = "5";
    private String m_filename = StringUtils.EMPTY;
    private String m_ipv6 = StringUtils.EMPTY;
    private String m_logFormat = DEFAULT_LOG_FORMAT;
    private String m_pathAdd = DEFAULT_PATH_ADD;
    private List<MRTGTarget> m_targets = new ArrayList<MRTGTarget>();
    private final List<String> m_hosts = new ArrayList<String>();
    private List<String> m_mibs = new ArrayList<String>();

    /**
     * @deprecated - only used from spring configuration
     */
    @Deprecated
    public MRTGConfig() {
    }

    public MRTGConfig(String cfgFileName) {
        m_filename = cfgFileName;
    }

    public void setFilename(String filename) {
        m_filename = filename;
    }

    public String getFilename() {
        return m_filename;
    }

    private String parseTargetId(String line) {
        return StringUtils.substringBetween(line, OPEN_BRACKET, CLOSE_BRACKET);
    }

    private String parseValue(String line) {
        return StringUtils.trim(StringUtils.substringAfter(line, MonitoringUtil.COLON));
    }

    private String parseSimpleEntry(String line) {
        String entry = StringUtils.trim(StringUtils.substringAfterLast(line, ":"));
        if (entry != null) {
            return entry;
        }
        return StringUtils.EMPTY;
    }

    private String parseHost(String line) {
        String delim = ")";
        String host = StringUtils.substringAfterLast(line, "@");
        if (StringUtils.isEmpty(host)) {
            host = "localhost";
        }
        if (host.endsWith(delim)) {
            host = StringUtils.substringBefore(host, delim);
        }
        return host;
    }

    public List<String> getMibs() {
        return m_mibs;
    }

    public void setMibs(List<String> mibs) {
        m_mibs = mibs;
    }

    public void setIPV6(String value) {
        m_ipv6 = value;
    }

    public String getIPV6() {
        return m_ipv6;
    }

    private void addLoadMibs(String mibs) {
        m_mibs.add(mibs);
    }

    private void addHostToList(String host) {
        if (!m_hosts.contains(host)) {
            m_hosts.add(host);
        }
    }

    public void parseConfig() throws IOException {
        m_targets.clear();
        m_mibs.clear();
        m_hosts.clear();
        String currentTargetGroup = StringUtils.EMPTY;
        BufferedReader reader = new BufferedReader(new FileReader(m_filename));
        for (String line = reader.readLine(); line != null; line = reader.readLine()) {
            if (line.startsWith("#target-group") || line.startsWith("# target-group")) {
                currentTargetGroup = StringUtils.substringAfterLast(line, "=");
            } else if (line.trim().startsWith(POUND)) {
                // this is a comment, ignore the contents of this line
                continue;
            } else if (line.startsWith(WORK_DIR_TOKEN)) {
                setWorkingDir(parseSimpleEntry(line));
            } else if (line.startsWith(THRESH_DIR_TOKEN)) {
                setThreshDir(parseSimpleEntry(line));
            } else if (line.startsWith(IPV6_TOKEN)) {
                setIPV6(parseSimpleEntry(line));
            } else if (line.startsWith(LOADMIBS_TOKEN)) {
                addLoadMibs(parseSimpleEntry(line));
            } else if (line.startsWith(LOG_FORMAT_TOKEN)) {
                setLogFormat(parseSimpleEntry(line));
            } else if (line.startsWith(PATH_ADD_TOKEN)) {
                setPathAdd(parseSimpleEntry(line));
            } else if (line.startsWith(RUN_AS_DAEMON_TOKEN)) {
                setRunAsDaemon(parseSimpleEntry(line));
            } else if (line.startsWith(NO_DETACH_TOKEN)) {
                setNoDetach(parseSimpleEntry(line));
            } else if (line.startsWith(INTERVAL_TOKEN)) {
                setInterval(parseSimpleEntry(line));
            } else {
                String targetId = parseTargetId(line);
                if (targetId != null) {
                    MRTGTarget target = getMRTGTarget(targetId, true);
                    String var = StringUtils.substringBefore(line, OPEN_BRACKET);
                    String value = parseValue(line);
                    if (var.equalsIgnoreCase(TARGET_TOKEN)) {
                        target.setExpression(value);
                        target.setHost(parseHost(line));
                        addHostToList(parseHost(line));
                        target.setGroup(currentTargetGroup);
                    } else if (var.equalsIgnoreCase(TITLE_TOKEN)) {
                        target.setTitle(value);
                    } else if (var.equalsIgnoreCase(PAGE_TOP_TOKEN)) {
                        target.setPageTop(value);
                    } else if (var.equalsIgnoreCase(MAX_BYTES_TOKEN)) {
                        target.setMaxBytes(value);
                    } else if (var.equalsIgnoreCase(SHORT_LEGEND_TOKEN)) {
                        target.setShortLegend(value);
                    } else if (var.equalsIgnoreCase(Y_LEGEND_TOKEN)) {
                        target.setYLegend(value);
                    } else if (var.equalsIgnoreCase(LEGEND_I_TOKEN)) {
                        target.setLegendI(value);
                    } else if (var.equalsIgnoreCase(LEGEND_O_TOKEN)) {
                        target.setLegendO(value);
                    } else if (var.equalsIgnoreCase(LEGEND_1_TOKEN)) {
                        target.setLegend1(value);
                    } else if (var.equalsIgnoreCase(LEGEND_2_TOKEN)) {
                        target.setLegend2(value);
                    } else if (var.equalsIgnoreCase(OPTIONS_TOKEN)) {
                        target.setOptions(value);
                    } else if (var.equalsIgnoreCase(UNSCALED_TOKEN)) {
                        target.setUnscaled(value);
                    } else if (var.equalsIgnoreCase(THRESH_MAX_TOKEN)) {
                        target.setThreshMax(value);
                    } else if (var.equalsIgnoreCase(THRESH_PROG_TOKEN)) {
                        target.setThreshProg(value);
                    } else if (var.equalsIgnoreCase(THRESH_PROG_OK_TOKEN)) {
                        target.setThreshProgOk(value);
                    } else if (var.equalsIgnoreCase(THRESH_DESC_TOKEN)) {
                        target.setThreshDesc(value);
                    } else if (var.equalsIgnoreCase(KMG_TOKEN)) {
                        // TODO Still need to handle this.
                        target.setkMG(value);
                    } else if (var.equalsIgnoreCase(FACTOR_TOKEN)) {
                        target.setFactor(value);
                    } else if (var.equalsIgnoreCase(KILO_TOKEN)) {
                        target.setKilo();
                    }
                }
            }
        }
        reader.close();
    }

    public List<MRTGTarget> getTargets() {
        return m_targets;
    }

    public void setTargets(List<MRTGTarget> targets) {
        m_targets = targets;
    }

    public void setWorkingDir(String dir) {
        m_workingDir = dir;
    }

    public String getWorkingDir() {
        return m_workingDir;
    }

    public void setThreshDir(String dir) {
        m_threshDir = dir;
    }

    public String getInterval() {
        return m_interval;
    }

    public void setInterval(String interval) {
        m_interval = interval;
    }

    public String getRunAsDaemon() {
        return m_runAsDaemon;
    }

    public String getThreshDir() {
        return m_threshDir;
    }

    public void setRunAsDaemon(String runAsDaemon) {
        m_runAsDaemon = runAsDaemon;
    }

    public String getNoDetach() {
        return m_noDetach;
    }

    public void setNoDetach(String noDetach) {
        m_noDetach = noDetach;
    }

    public List<String> getHosts() {
        return m_hosts;
    }

    public String getLogFormat() {
        return m_logFormat;
    }

    public void setLogFormat(String logFormat) {
        m_logFormat = logFormat;
    }

    public String getPathAdd() {
        return m_pathAdd;
    }

    public void setPathAdd(String pathAdd) {
        m_pathAdd = pathAdd;
    }

    private MRTGTarget getMRTGTarget(String targetId, boolean create) {
        MRTGTarget result = null;
        for (MRTGTarget target : m_targets) {
            if (target.getId().equals(targetId)) {
                result = target;
                break;
            }
        }
        if (result == null && create) {
            result = new MRTGTarget();
            m_targets.add(result);
            result.setId(targetId);
        }
        return result;
    }

    private void dumpTarget(StringBuilder buf, MRTGTarget target) {
        if (target.getId().equals(MonitoringUtil.UNDERSCORE)) {
            buf.append(EOL);
            buf.append(POUND + EOL);
            buf.append("# Default settings" + EOL);
            buf.append(POUND + EOL);
        } else {
            buf.append(EOL);
            buf.append(POUND + EOL);
            buf.append("# " + target.getTitle() + EOL);
            buf.append(POUND + EOL);
            appendTargetToken(buf, TARGET_TOKEN, target.getExpression(), target.getId());
            appendTargetToken(buf, TITLE_TOKEN, target.getTitle(), target.getId());
            appendTargetToken(buf, PAGE_TOP_TOKEN, target.getPageTop(), target.getId());
            appendTargetToken(buf, MAX_BYTES_TOKEN, String.valueOf(target.getMaxBytes()), target
                    .getId());
            appendTargetToken(buf, SHORT_LEGEND_TOKEN, target.getShortLegend(), target.getId());
            appendTargetToken(buf, Y_LEGEND_TOKEN, target.getYLegend(), target.getId());
            appendTargetToken(buf, LEGEND_I_TOKEN, target.getLegendI(), target.getId());
            appendTargetToken(buf, LEGEND_O_TOKEN, target.getLegendO(), target.getId());
            appendTargetToken(buf, LEGEND_1_TOKEN, target.getLegend1(), target.getId());
            appendTargetToken(buf, LEGEND_2_TOKEN, target.getLegend2(), target.getId());
            appendTargetToken(buf, UNSCALED_TOKEN, target.getUnscaled(), target.getId());
            appendTargetToken(buf, THRESH_MAX_TOKEN, target.getThreshMax(), target.getId());
            appendTargetToken(buf, THRESH_PROG_TOKEN, target.getThreshProg(), target.getId());
            appendTargetToken(buf, THRESH_PROG_OK_TOKEN, target.getThreshProgOk(), target.getId());
            appendTargetToken(buf, THRESH_DESC_TOKEN, target.getThreshDesc(), target.getId());
        }
        if (target.gauge() || target.growRight() || target.noPercent()) {
            buf.append(OPTIONS_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                    + MonitoringUtil.COLON + MonitoringUtil.SPACE);
            buf.append(getTargetPropertiesAsString(target));
            buf.append(EOL);
        }
        buf.append(EOL);
    }

    private String getTargetPropertiesAsString(MRTGTarget target) {
        List<String> targetProperties = new ArrayList<String>();
        if (target.gauge()) {
            targetProperties.add("gauge");
        }
        if (target.growRight()) {
            targetProperties.add("growright");
        }
        if (target.noPercent()) {
            targetProperties.add("nopercent");
        }
        if (target.bits()) {
            targetProperties.add("bits");
        }
        if (target.transparent()) {
            targetProperties.add("transparent");
        }
        if (target.perMinute()) {
            targetProperties.add("perminute");
        }
        return StringUtils.join(targetProperties, COMMA);
    }

    private void appendTargetToken(StringBuilder buf, String token, String value, String targetId) {
        if (!StringUtils.isEmpty(value)) {
            buf.append(token + OPEN_BRACKET + targetId + CLOSE_BRACKET + MonitoringUtil.COLON
                    + MonitoringUtil.SPACE + value + EOL);
        }
    }

    @Override
    public String toString() {
        StringBuilder buf = new StringBuilder();
        dumpHeader(buf);
        buf.append(EOL);
        String currentGroup = StringUtils.EMPTY;
        for (MRTGTarget target : getTargets()) {
            String group = target.getGroup();
            if (!group.equals(currentGroup)) {
                buf.append("#target-group=" + group + EOL + EOL);
                currentGroup = group;
            }
            dumpTarget(buf, target);
        }
        return buf.toString();
    }

    private void dumpHeader(StringBuilder buf) {
        appendTokenValue(buf, RUN_AS_DAEMON_TOKEN, getRunAsDaemon());
        appendTokenValue(buf, NO_DETACH_TOKEN, getNoDetach());
        appendTokenValue(buf, INTERVAL_TOKEN, getInterval());
        appendTokenValue(buf, WORK_DIR_TOKEN, getWorkingDir());
        appendTokenValue(buf, THRESH_DIR_TOKEN, getThreshDir());
        for (String mib : m_mibs) {
            appendTokenValue(buf, LOADMIBS_TOKEN, mib);
        }
        appendTokenValue(buf, IPV6_TOKEN, getIPV6());
        appendTokenValue(buf, LOG_FORMAT_TOKEN, getLogFormat());
        appendTokenValue(buf, PATH_ADD_TOKEN, getPathAdd());
    }

    private void appendTokenValue(StringBuilder buffer, String token, String value) {
        if (!StringUtils.isEmpty(value)) {
            buffer.append(token + MonitoringUtil.COLON + MonitoringUtil.SPACE + value + EOL);
        }
    }

    public boolean writeFile() {
        try {
            BufferedWriter mrtgFileWriter = new BufferedWriter(new FileWriter(m_filename));
            mrtgFileWriter.write(toString());
            mrtgFileWriter.flush();
            mrtgFileWriter.close();
        } catch (IOException e) {
            return false;
        }
        return true;
    }
}
