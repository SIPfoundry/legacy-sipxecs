/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.alarm;

import static java.lang.String.format;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.RunRequest;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class AlarmHistoryManagerImpl implements AlarmHistoryManager {
    private static final String ERROR_IO_EXCEPTION = "&error.io.exception";
    private static final Log LOG = LogFactory.getLog(AlarmHistoryManagerImpl.class);
    private ConfigManager m_configManager;
    private LocationsManager m_locationsManager;
    private File m_logDirectory;

    public List<AlarmEvent> getAlarmEvents(String host, Date startDate, Date endDate) {
        return getAlarmEventsByPage(host, startDate, endDate, 0, Integer.MAX_VALUE);
    }

    public List<AlarmEvent> getAlarmEventsByPage(String host, Date startDate, Date endDate, int first, int pageSize) {
        AlarmLogParser parser = new AlarmLogParser();
        InputStream in = null;
        try {
            in = getAlarmDataStream(host);
            List<AlarmEvent> events;
            if (in == null) {
                events = Collections.emptyList();
            } else {
                events = parser.parse(startDate, endDate, first, pageSize, in);
            }
            return events;
        } catch (IOException ex) {
            throw new UserException(ERROR_IO_EXCEPTION, ex.getMessage());
        } finally {
            IOUtils.closeQuietly(in);
        }
    }

    private InputStream getAlarmDataStream(String host) throws IOException {
        refreshLogs();
        File log = new File(m_logDirectory, format("snmptrapd-%s.log", host));
        if (!log.exists()) {
            LOG.warn("snmp alarm file not found " + log.getAbsolutePath());
            return null;
        }
        return new FileInputStream(log);
    }

    void refreshLogs() {
        Location l = m_locationsManager.getPrimaryLocation();
        RunRequest getLogs = new RunRequest("alarm logs", Collections.singleton(l));
        getLogs.setBundles("upload_alarm_log");
        m_configManager.run(getLogs);
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setLogDirectory(String logDirectory) {
        m_logDirectory = new File(logDirectory);
    }
}
