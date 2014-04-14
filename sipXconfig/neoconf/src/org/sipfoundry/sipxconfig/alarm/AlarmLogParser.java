/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.alarm;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class AlarmLogParser {
    private static final Log LOG = LogFactory.getLog(AlarmLogParser.class);
    private static final Pattern ALARM_LINE = Pattern.compile("(\\S+)::(\\S+)");
    private static final Pattern DATE_REGEXP = Pattern.compile("\\d{4}?-\\d{2}?-\\d{2}?");
    private static final DateFormat DATE_FORMAT = new SimpleDateFormat("yyyy-MM-ddHH:mm:ss");

    public List<AlarmEvent> parse(Date after, Date before, int first, int max, InputStream in) throws IOException {
        BufferedReader input = new BufferedReader(new InputStreamReader(in, "UTF-8"));
        String line = "";
        List<AlarmEvent> alarms = new ArrayList<AlarmEvent>();
        Date when = null;
        int i = 0;
        while (alarms.size() < max && line != null) {
            line = input.readLine();
            if (StringUtils.isBlank(line)) {
                continue;
            }
            String[] fields = line.split(" ");
            if (fields.length == 0) {
                continue;
            }

            if (fields[0].equals("NET-SNMP")) {
                continue;
            }

            if (fields[0].startsWith("DISMAN-EVENT-MIB::")) {
                if (before != null && when.after(before)) {
                    break;
                }
                if (after == null || when.after(after)) {
                    i++;
                    if (i > first) {
                        AlarmEvent e = parseEvent(when, line);
                        if (e != null) {
                            alarms.add(e);
                        }
                    }
                }
                continue;
            }

            if (DATE_REGEXP.matcher(fields[0]).find()) {
                try {
                    when = DATE_FORMAT.parse(fields[0] + fields[1]);
                } catch (ParseException e) {
                    LOG.error("Bad date", e);
                }
            }
        }
        return alarms;
    }

    AlarmEvent parseEvent(Date when, String line) {
        Map<String, String> fields = parseFields("SIPXECS-ALARM-NOTIFICATION-MIB", line);
        if (fields.isEmpty()) {
            return null;
        }
        AlarmDefinition def = new AlarmDefinition(fields.get("sipxecsAlarmId"));
        String msg = fields.get("sipxecsAlarmDescr");
        AlarmEvent e = new AlarmEvent(when, def, msg);
        return e;
    }

    Map<String, String> parseFields(String forMib, String line) {
        Map<String, String> map = new HashMap<String, String>();
        Matcher m = ALARM_LINE.matcher(line);
        boolean more;
        String key = null;
        String mib = null;
        int valueStart = 0;
        do {
            more = m.find();
            boolean first = (key == null);
            if (!first) {
                if (mib.equals(forMib)) {
                    int valueEnd = (more ? m.start() : line.length());
                    String encoded = line.substring(valueStart, valueEnd).trim();
                    String value = decodeValue(encoded);
                    map.put(key, value);
                }
            }
            if (more) {
                mib = m.group(1);
                key = m.group(2);
                valueStart = m.end();
            }
        } while (more);
        return map;
    }

    String decodeValue(String encoded) {
        String[] split = StringUtils.split(encoded, ":", 2);
        if (split.length == 2) {
            if (split[0].equals("= STRING")) {
                String v = split[1].trim();
                if (StringUtils.isNotBlank(v)) {
                    if (v.charAt(0) == '"' && v.charAt(v.length() - 1) == '"') {
                        v = v.substring(1, v.length() - 1);
                    }
                }
                return v;
            }
        }
        return null;
    }
}
