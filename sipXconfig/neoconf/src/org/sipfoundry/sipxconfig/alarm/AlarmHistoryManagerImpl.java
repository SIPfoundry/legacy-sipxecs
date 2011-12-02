/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpException;
import org.apache.commons.httpclient.methods.GetMethod;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

public class AlarmHistoryManagerImpl implements AlarmHistoryManager {

    private static final String ERROR_IO_EXCEPTION = "&error.io.exception";
    private static final String ALARMS_LOG = "sipXalarms.log";

    private String m_logDirectory;
    private LocationsManager m_locationsManager;

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setLogDirectory(String logDirectory) {
        m_logDirectory = logDirectory;
    }

    public List<AlarmEvent> getAlarmEvents(String host, Date startDate, Date endDate) {
        try {
            return parseEventsStream(getAlarmDataStream(host), startDate, endDate);
        } catch (IOException ex) {
            throw new UserException(ERROR_IO_EXCEPTION, ex.getMessage());
        }
    }

    public List<AlarmEvent> getAlarmEventsByPage(String host, Date startDate, Date endDate, int first, int pageSize) {
        try {
            return parseEventsStreamByPage(getAlarmDataStream(host), startDate, endDate, first, pageSize);
        } catch (IOException ex) {
            throw new UserException(ERROR_IO_EXCEPTION, ex.getMessage());
        }
    }

    private InputStream getAlarmDataStream(String host) throws IOException {
        GetMethod httpget = null;
        try {
            HttpClient client = new HttpClient();
            Location location = m_locationsManager.getLocationByFqdn(host);
            httpget = new GetMethod(location.getHttpsServerUrl() + m_logDirectory + "/" + ALARMS_LOG);
            int statusCode = client.executeMethod(httpget);
            if (statusCode != 200) {
                throw new UserException("&error.https.server.status.code", host, String.valueOf(statusCode));
            }
            byte[] bytes = httpget.getResponseBody();
            return new ByteArrayInputStream(bytes);
        } catch (HttpException ex) {
            throw new UserException("&error.https.server", host, ex.getMessage());
        } catch (XmlRpcRemoteException ex) {
            throw new UserException("&error.xml.rpc", ex.getMessage(), host);
        } finally {
            if (httpget != null) {
                httpget.releaseConnection();
            }
        }
    }

    protected List<AlarmEvent> parseEventsStream(InputStream responseStream, Date startDate, Date endDate)
        throws IOException {
        BufferedReader input = new BufferedReader(new InputStreamReader(responseStream, "UTF-8"));
        String line = null;
        List<AlarmEvent> contents = new ArrayList<AlarmEvent>();
        while ((line = input.readLine()) != null) {
            AlarmEvent alarmEvent = new AlarmEvent(line);
            Date date = alarmEvent.getDate();
            if (startDate.before(date) && endDate.after(date)) {
                contents.add(alarmEvent);
            }
        }
        return contents;
    }

    protected List<AlarmEvent> parseEventsStreamByPage(InputStream responseStream, Date startDate, Date endDate,
            int first, int pageSize) throws IOException {
        List<AlarmEvent> contents = parseEventsStream(responseStream, startDate, endDate);
        return DataCollectionUtil.getPage(contents, first, pageSize);
    }
}
