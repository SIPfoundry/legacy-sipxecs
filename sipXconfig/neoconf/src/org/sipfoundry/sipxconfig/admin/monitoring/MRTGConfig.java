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
import java.util.StringTokenizer;

import org.apache.commons.lang.StringUtils;

public class MRTGConfig {
    private static final String WORK_DIR_TOKEN = "workdir";
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
    private static final String LIB_ADD_TOKEN = "LibAdd";
    private static final String DEFAULT_LOG_FORMAT = "rrdtool";
    private static final String DEFAULT_PATH_ADD = "/usr/bin";
    private static final String DEFAULT_LIB_ADD = "/usr/lib/perl5/5.8.5/i386-linux-thread-multi";

    private static final String COMMA = ",";
    private static final String POUND = "#";
    private static final String OPEN_BRACKET = "[";
    private static final String CLOSE_BRACKET = "]";
    private static final String EXPECTED = "Expected";

    private String m_workingDir;
    private String m_filename = StringUtils.EMPTY;
    private String m_ipv6 = StringUtils.EMPTY;
    private String m_logFormat = DEFAULT_LOG_FORMAT;
    private String m_pathAdd = DEFAULT_PATH_ADD;
    private String m_libAdd = DEFAULT_LIB_ADD;
    private List<MRTGTarget> m_targets = new ArrayList<MRTGTarget>();
    private List<String> m_hosts = new ArrayList<String>();
    private List<String> m_mibs = new ArrayList<String>();

    public MRTGConfig(String cfgFileName) {
        m_filename = cfgFileName;
    }

    public String getFilename() {
        return m_filename;
    }

    private String parseTargetId(StringTokenizer st, int lineNo) throws Exception {
        String result = StringUtils.EMPTY;
        if (!st.hasMoreTokens()) {
            throw new MRTGParserException(m_filename + MonitoringUtil.COLON + lineNo
                    + MonitoringUtil.COLON + MonitoringUtil.SPACE + EXPECTED
                    + MonitoringUtil.SPACE + OPEN_BRACKET);
        }
        String d = st.nextToken();
        if (!d.equals(OPEN_BRACKET)) {
            throw new MRTGParserException(m_filename + MonitoringUtil.COLON + lineNo
                    + MonitoringUtil.COLON + MonitoringUtil.SPACE + EXPECTED
                    + MonitoringUtil.SPACE + OPEN_BRACKET);
        }
        if (!st.hasMoreTokens()) {
            throw new MRTGParserException(m_filename + MonitoringUtil.COLON + lineNo
                    + ": Missing target name");
        }
        result = st.nextToken();
        if (result.equals(OPEN_BRACKET) || result.equals(CLOSE_BRACKET)) {
            throw new MRTGParserException(m_filename + MonitoringUtil.COLON + lineNo
                    + ": Unexpected token: " + result);
        }
        if (!st.hasMoreTokens()) {
            throw new MRTGParserException(m_filename + MonitoringUtil.COLON + lineNo
                    + MonitoringUtil.COLON + MonitoringUtil.SPACE + EXPECTED
                    + MonitoringUtil.SPACE + CLOSE_BRACKET);
        }
        d = st.nextToken();
        if (!d.equals(CLOSE_BRACKET)) {
            throw new MRTGParserException(m_filename + MonitoringUtil.COLON + lineNo
                    + MonitoringUtil.COLON + MonitoringUtil.SPACE + EXPECTED
                    + MonitoringUtil.SPACE + CLOSE_BRACKET);
        }
        return result;
    }

    private String parseValue(StringTokenizer st, int lineNo) throws Exception {
        StringBuffer result = new StringBuffer();
        if (!st.hasMoreTokens()) {
            throw new MRTGParserException(m_filename + MonitoringUtil.COLON + lineNo
                    + MonitoringUtil.COLON + MonitoringUtil.SPACE + EXPECTED
                    + MonitoringUtil.SPACE + MonitoringUtil.COLON);
        }
        String d = st.nextToken();
        if (!d.equals(MonitoringUtil.COLON)) {
            throw new MRTGParserException(m_filename + MonitoringUtil.COLON + lineNo
                    + MonitoringUtil.COLON + MonitoringUtil.SPACE + EXPECTED
                    + MonitoringUtil.SPACE + MonitoringUtil.COLON);
        }
        while (st.hasMoreTokens()) {
            result.append(st.nextToken());
        }
        return result.toString().trim();
    }

    private String parseSimpleEntry(String line) throws Exception {
        StringTokenizer st = new StringTokenizer(line, ":");
        if (st.hasMoreTokens()) {
            st.nextToken();
            if (st.hasMoreTokens()) {
                return st.nextToken().trim();
            }
        }
        return "";
    }

    private String parseHost(String line) {
        String host = line.substring(line.lastIndexOf("@") + 1, line.length());
        if (host == null || host.trim().equals(StringUtils.EMPTY)) {
            host = "localhost";
        }
        if (host.endsWith(")")) {
            host = host.substring(0, host.length() - 1);
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

    public void parseConfig() throws Exception {
        m_targets.clear();
        m_mibs.clear();
        String currentTargetGroup = StringUtils.EMPTY;
        BufferedReader reader = new BufferedReader(new FileReader(m_filename));
        int lineNo = 1;
        for (String line = reader.readLine(); line != null; line = reader.readLine()) {
            if (line.startsWith("#target-group") || line.startsWith("# target-group")) {
                StringTokenizer st = new StringTokenizer(line, "=");
                if (st.hasMoreTokens()) {
                    st.nextToken();
                    if (st.hasMoreTokens()) {
                        currentTargetGroup = st.nextToken();
                    }
                }
            }

            if (line.startsWith(WORK_DIR_TOKEN)) {
                setWorkingDir(parseSimpleEntry(line));
            } else if (line.startsWith(IPV6_TOKEN)) {
                setIPV6(parseSimpleEntry(line));
            } else if (line.startsWith(LOADMIBS_TOKEN)) {
                addLoadMibs(parseSimpleEntry(line));
            } else if (line.startsWith(LOG_FORMAT_TOKEN)) {
                setLogFormat(parseSimpleEntry(line));
            } else if (line.startsWith(PATH_ADD_TOKEN)) {
                setPathAdd(parseSimpleEntry(line));
            } else if (line.startsWith(LIB_ADD_TOKEN)) {
                setLibAdd(parseSimpleEntry(line));
            } else {
                StringTokenizer st = new StringTokenizer(line, ":[]", true);
                if (st.hasMoreTokens()) {
                    String var = st.nextToken();
                    if (var.equalsIgnoreCase(TARGET_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setExpression(parseValue(st, lineNo));
                        target.setHost(parseHost(line));
                        addHostToList(parseHost(line));
                        target.setGroup(currentTargetGroup);
                    } else if (var.equalsIgnoreCase(TITLE_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setTitle(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(PAGE_TOP_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setPageTop(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(MAX_BYTES_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setMaxBytes(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(SHORT_LEGEND_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setShortLegend(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(Y_LEGEND_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setYLegend(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(LEGEND_I_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setLegendI(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(LEGEND_O_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setLegendO(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(LEGEND_1_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setLegend1(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(LEGEND_2_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setLegend2(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(OPTIONS_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setOptions(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(UNSCALED_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setUnscaled(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(KMG_TOKEN)) {
                        // TODO Still need to handle this.
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setkMG(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(FACTOR_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
                        target.setFactor(parseValue(st, lineNo));
                    } else if (var.equalsIgnoreCase(KILO_TOKEN)) {
                        String targetId = parseTargetId(st, lineNo);
                        MRTGTarget target = getMRTGTarget(targetId, true);
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

    public List<String> getHosts() {
        return m_hosts;
    }

    public String getLibAdd() {
        return m_libAdd;
    }

    public void setLibAdd(String libAdd) {
        m_libAdd = libAdd;
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

    private void dumpTarget(StringBuffer buf, String eol, MRTGTarget target) {
        if (target.getId().equals(MonitoringUtil.UNDERSCORE)) {
            buf.append(eol);
            buf.append(POUND + eol);
            buf.append("# Default settings" + eol);
            buf.append(POUND + eol);
        } else {
            buf.append(eol);
            buf.append(POUND + eol);
            buf.append("# " + target.getTitle() + eol);
            buf.append(POUND + eol);
            if (target.getExpression() != null && target.getExpression().length() != 0) {
                buf.append(TARGET_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                        + MonitoringUtil.COLON + MonitoringUtil.SPACE + target.getExpression()
                        + eol);
            }
            if (target.getTitle() != null && target.getTitle().length() != 0) {
                buf.append(TITLE_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                        + MonitoringUtil.COLON + MonitoringUtil.SPACE + target.getTitle() + eol);
            }
            if (target.getPageTop() != null && target.getPageTop().length() != 0) {
                buf
                        .append(PAGE_TOP_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                                + MonitoringUtil.COLON + MonitoringUtil.SPACE
                                + target.getPageTop() + eol);
            }
            if (String.valueOf(target.getMaxBytes()).length() != 0) {
                buf.append(MAX_BYTES_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                        + MonitoringUtil.COLON + MonitoringUtil.SPACE + target.getMaxBytes()
                        + eol);
            }
            if (target.getShortLegend() != null && target.getShortLegend().length() != 0) {
                buf.append(SHORT_LEGEND_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                        + MonitoringUtil.COLON + MonitoringUtil.SPACE + target.getShortLegend()
                        + eol);
            }
            if (target.getYLegend() != null && target.getYLegend().length() != 0) {
                buf
                        .append(Y_LEGEND_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                                + MonitoringUtil.COLON + MonitoringUtil.SPACE
                                + target.getYLegend() + eol);
            }
            if (target.getLegendI() != null && target.getLegendI().length() != 0) {
                buf
                        .append(LEGEND_I_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                                + MonitoringUtil.COLON + MonitoringUtil.SPACE
                                + target.getLegendI() + eol);
            }
            if (target.getLegendO() != null && target.getLegendO().length() != 0) {
                buf
                        .append(LEGEND_O_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                                + MonitoringUtil.COLON + MonitoringUtil.SPACE
                                + target.getLegendO() + eol);
            }
            if (target.getLegend1() != null && target.getLegend1().length() != 0) {
                buf
                        .append(LEGEND_1_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                                + MonitoringUtil.COLON + MonitoringUtil.SPACE
                                + target.getLegend1() + eol);
            }
            if (target.getLegend2() != null && target.getLegend2().length() != 0) {
                buf
                        .append(LEGEND_2_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                                + MonitoringUtil.COLON + MonitoringUtil.SPACE
                                + target.getLegend2() + eol);
            }
            if (target.getUnscaled() != null && target.getUnscaled().length() != 0) {
                buf.append(UNSCALED_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                        + MonitoringUtil.COLON + MonitoringUtil.SPACE + target.getUnscaled()
                        + eol);
            }
        }
        if (target.gauge() || target.growRight() || target.noPercent()) {
            boolean comma = false;
            buf.append(OPTIONS_TOKEN + OPEN_BRACKET + target.getId() + CLOSE_BRACKET
                    + MonitoringUtil.COLON + MonitoringUtil.SPACE);
            if (target.gauge()) {
                if (comma) {
                    buf.append(COMMA);
                }
                buf.append("gauge");
                comma = true;
            }
            if (target.growRight()) {
                if (comma) {
                    buf.append(COMMA);
                }
                buf.append("growright");
                comma = true;
            }
            if (target.noPercent()) {
                if (comma) {
                    buf.append(COMMA);
                }
                buf.append("nopercent");
                comma = true;
            }
            if (target.bits()) {
                if (comma) {
                    buf.append(COMMA);
                }
                buf.append("bits");
                comma = true;
            }
            if (target.transparent()) {
                if (comma) {
                    buf.append(COMMA);
                }
                buf.append("transparent");
                comma = true;
            }
            if (target.perMinute()) {
                if (comma) {
                    buf.append(COMMA);
                }
                buf.append("perminute");
                comma = true;
            }
            buf.append(eol);
        }
        buf.append(eol);
    }

    public String toString() {
        StringBuffer buf = new StringBuffer();
        String eol = System.getProperty("line.separator");

        if (!getWorkingDir().equals(StringUtils.EMPTY)) {
            buf.append(WORK_DIR_TOKEN + MonitoringUtil.COLON + MonitoringUtil.SPACE
                    + getWorkingDir() + eol);
        }
        for (int i = 0; i < m_mibs.size(); i++) {
            buf.append(LOADMIBS_TOKEN + MonitoringUtil.COLON + MonitoringUtil.SPACE
                    + m_mibs.get(i) + eol);
        }
        if (!getIPV6().equals(StringUtils.EMPTY)) {
            buf
                    .append(IPV6_TOKEN + MonitoringUtil.COLON + MonitoringUtil.SPACE + getIPV6()
                            + eol);
        }
        if (!getLogFormat().equals(StringUtils.EMPTY)) {
            buf.append(LOG_FORMAT_TOKEN + MonitoringUtil.COLON + MonitoringUtil.SPACE
                    + getLogFormat() + eol);
        }
        if (!getPathAdd().equals(StringUtils.EMPTY)) {
            buf.append(PATH_ADD_TOKEN + MonitoringUtil.COLON + MonitoringUtil.SPACE
                    + getPathAdd() + eol);
        }
        if (!getLibAdd().equals(StringUtils.EMPTY)) {
            buf.append(LIB_ADD_TOKEN + MonitoringUtil.COLON + MonitoringUtil.SPACE + getLibAdd()
                    + eol);
        }
        buf.append(eol);
        String currentGroup = StringUtils.EMPTY;
        for (MRTGTarget target : getTargets()) {
            String group = target.getGroup();
            if (!group.equals(currentGroup)) {
                buf.append("#target-group=" + group + eol + eol);
                currentGroup = group;
            }
            dumpTarget(buf, eol, target);
        }
        return buf.toString();
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

    private class MRTGParserException extends Exception {

        public MRTGParserException(String text) {
            super(text);
        }
    }
}
