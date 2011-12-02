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

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

public class MRTGTarget implements PrimaryKeySource {
    private static final Log LOG = LogFactory.getLog(MRTGTarget.class);
    private String m_id = StringUtils.EMPTY;
    private String m_group = StringUtils.EMPTY;
    private String m_expression = StringUtils.EMPTY;
    private String m_title = StringUtils.EMPTY;
    private String m_pageTop = StringUtils.EMPTY;
    private String m_shortLegend = StringUtils.EMPTY;
    private String m_yLegend = StringUtils.EMPTY;
    private String m_legendI = StringUtils.EMPTY;
    private String m_legendO = StringUtils.EMPTY;
    private String m_legend1 = StringUtils.EMPTY;
    private String m_legend2 = StringUtils.EMPTY;
    private String m_unscaled = StringUtils.EMPTY;
    private String m_threshMax = StringUtils.EMPTY;
    private String m_threshProg = StringUtils.EMPTY;
    private String m_threshProgOk = StringUtils.EMPTY;
    private String m_threshDesc = StringUtils.EMPTY;
    private String m_kMG = StringUtils.EMPTY;
    private String m_host = "localhost";
    private long m_maxBytes = 10000000000L;
    private int m_factor = 1;
    private int m_kilo;
    private boolean m_gauge;
    private boolean m_growRight;
    private boolean m_noPercent;
    private boolean m_bits;
    private boolean m_transparent;
    private boolean m_perMinute;
    private String m_logFormat = StringUtils.EMPTY;
    private String m_pathAdd = StringUtils.EMPTY;

    public String getId() {
        return m_id;
    }

    public void setId(String id) {
        m_id = id;
    }

    public String getGroup() {
        return m_group;
    }

    public void setGroup(String group) {
        m_group = group;
    }

    public String getExpression() {
        return m_expression;
    }

    public void setExpression(String expression) {
        m_expression = expression.trim();
    }

    public String getTitle() {
        return m_title;
    }

    public void setTitle(String title) {
        m_title = title.trim();
    }

    public String getPageTop() {
        return m_pageTop;
    }

    public void setPageTop(String pageTop) {
        m_pageTop = pageTop.trim();
    }

    public String getShortLegend() {
        return m_shortLegend;
    }

    public void setShortLegend(String shortLegend) {
        m_shortLegend = shortLegend.trim();
    }

    public String getYLegend() {
        return m_yLegend;
    }

    public void setYLegend(String yLegend) {
        m_yLegend = yLegend.trim();
    }

    public String getLegendI() {
        return m_legendI;
    }

    public void setLegendI(String legendI) {
        m_legendI = legendI.trim();
    }

    public String getLegendO() {
        return m_legendO;
    }

    public void setLegendO(String legendO) {
        m_legendO = legendO.trim();
    }

    public String getLegend1() {
        return m_legend1;
    }

    public void setLegend1(String legend1) {
        m_legend1 = legend1.trim();
    }

    public String getLegend2() {
        return m_legend2;
    }

    public void setLegend2(String legend2) {
        m_legend2 = legend2.trim();
    }

    public String getUnscaled() {
        return m_unscaled;
    }

    public void setUnscaled(String unscaled) {
        m_unscaled = unscaled.trim();
    }

    public String getThreshMax() {
        return m_threshMax;
    }

    public void setThreshMax(String value) {
        m_threshMax = value.trim();
    }

    public String getThreshProg() {
        return m_threshProg;
    }

    public void setThreshProg(String value) {
        m_threshProg = value.trim();
    }

    public String getThreshProgOk() {
        return m_threshProgOk;
    }

    public void setThreshProgOk(String value) {
        m_threshProgOk = value.trim();
    }

    public String getThreshDesc() {
        return m_threshDesc;
    }

    public void setThreshDesc(String value) {
        m_threshDesc = value.trim();
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

    public void setOptions(String options) {
        String[] targetOptions = StringUtils.split(options, ",");
        for (String option : targetOptions) {
            option = StringUtils.trim(option);
            if (option.equalsIgnoreCase("gauge")) {
                m_gauge = true;
            } else if (option.equalsIgnoreCase("growright")) {
                m_growRight = true;
            } else if (option.equalsIgnoreCase("nopercent")) {
                m_noPercent = true;
            } else if (option.equalsIgnoreCase("bits")) {
                m_bits = true;
            } else if (option.equalsIgnoreCase("transparent")) {
                m_transparent = true;
            } else if (option.equalsIgnoreCase("perminute")) {
                m_perMinute = true;
            }
        }
    }

    public long getMaxBytes() {
        return m_maxBytes;
    }

    public void setMaxBytes(String maxBytesString) {
        // don't even try to parse it if the string is empty
        if (StringUtils.isEmpty(maxBytesString)) {
            return;
        }
        try {
            long value = Long.parseLong(maxBytesString);
            if (value > 0 && value < Integer.MAX_VALUE) {
                setMaxBytes(value);
            }
        } catch (NumberFormatException e) {
            // ignore max bytes if not valid number.
            LOG.warn(e);
        }
    }

    public void setMaxBytes(long value) {
        if (value > 0) {
            m_maxBytes = value;
        }
    }

    public int getFactor() {
        return m_factor;
    }

    public void setFactor(String factorString) {
        if (StringUtils.isEmpty(factorString)) {
            // don't even try to parse it if the string is empty
            return;
        }
        try {
            m_factor = Integer.parseInt(factorString);
        } catch (NumberFormatException e) {
            // ignore max bytes if not valid number.
            LOG.warn(e);
        }
    }

    public void setFactor(int value) {
        if (value > 0) {
            m_factor = value;
        }
    }

    public int getKilo() {
        return m_kilo;
    }

    public void setKilo() {
        m_kilo = 1024;
    }

    public String getkMG() {
        return m_kMG;
    }

    public void setkMG(String value) {
        m_kMG = value;
    }

    public boolean gauge() {
        return m_gauge;
    }

    public boolean growRight() {
        return m_growRight;
    }

    public boolean noPercent() {
        return m_noPercent;
    }

    public boolean bits() {
        return m_bits;
    }

    public boolean transparent() {
        return m_transparent;
    }

    public boolean perMinute() {
        return m_perMinute;
    }

    public String getHost() {
        return m_host;
    }

    public void setHost(String host) {
        m_host = host;
    }

    public Object getPrimaryKey() {
        return getTitle();
    }
}
