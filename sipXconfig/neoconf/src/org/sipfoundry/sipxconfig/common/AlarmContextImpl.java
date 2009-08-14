/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpException;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.alarm.Alarm;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmConfiguration;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmEvent;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServer;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerActivatedEvent;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerConfiguration;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerContacts;
import org.sipfoundry.sipxconfig.admin.commserver.AlarmApi;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public class AlarmContextImpl extends SipxHibernateDaoSupport implements AlarmContext, ApplicationListener {
    private static final String ERROR_IO_EXCEPTION = "&error.io.exception";
    private static final Log LOG = LogFactory.getLog(AlarmContextImpl.class);
    private static final String ALARMS_LOG = "sipXalarms.log";
    private static final String DEFAULT_HOST = "@localhost";
    private static final String EMPTY = "";
    private ApiProvider<AlarmApi> m_alarmApiProvider;
    private SipxReplicationContext m_replicationContext;
    private AlarmServerConfiguration m_alarmServerConfiguration;
    private AlarmConfiguration m_alarmsConfiguration;
    private String m_sipxUser;
    private String m_logDirectory;
    private String m_configDirectory;
    private String m_alarmsStringsDirectory;
    private LocationsManager m_locationsManager;

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setAlarmApiProvider(ApiProvider<AlarmApi> alarmApiProvider) {
        m_alarmApiProvider = alarmApiProvider;
    }

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setAlarmServerConfiguration(AlarmServerConfiguration alarmServerConfiguration) {
        m_alarmServerConfiguration = alarmServerConfiguration;
    }

    @Required
    public void setAlarmsConfiguration(AlarmConfiguration alarmConfiguration) {
        m_alarmsConfiguration = alarmConfiguration;
    }

    public String getSipxUser() {
        return m_sipxUser;
    }

    public void setSipxUser(String sipxUser) {
        m_sipxUser = sipxUser;
    }

    public void setLogDirectory(String logDirectory) {
        m_logDirectory = logDirectory;
    }

    public String getLogDirectory() {
        return m_logDirectory;
    }

    public String getConfigDirectory() {
        return m_configDirectory;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public String getAlarmsStringsDirectory() {
        return m_alarmsStringsDirectory;
    }

    public void setAlarmsStringsDirectory(String alarmsStringsDirectory) {
        m_alarmsStringsDirectory = alarmsStringsDirectory;
    }

    public void raiseAlarm(String alarmId, String... alarmParams) {
        try {
            getAlarmApi().raiseAlarm(getHost(), alarmId, alarmParams);
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }

    public void reloadAlarms() {
        try {
            getAlarmApi().reloadAlarms(getHost());
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof AlarmServerActivatedEvent) {
            // send "reloadAlarms" command to the supervisor
            reloadAlarms();
        }
    }

    public void deployAlarmConfiguration(AlarmServer server, List<Alarm> alarms) {
        // save alarm server configuration
        saveAlarmServer(server);
        // replicate new alarm server configuration
        replicateAlarmServer(m_replicationContext, null);
        // replicate new alarm types configuration
        replicateAlarmsConfiguration(alarms);

        m_replicationContext.publishEvent(new AlarmServerActivatedEvent(this));
    }

    public AlarmServer getAlarmServer() {
        List servers = getHibernateTemplate().loadAll(AlarmServer.class);
        AlarmServer server = (AlarmServer) DataAccessUtils.singleResult(servers);
        if (server == null) {
            server = newAlarmServer();
        }

        return server;
    }

    private AlarmServer newAlarmServer() {
        AlarmServer server = new AlarmServer();
        // set email notification enabled by default
        server.setEmailNotificationEnabled(true);
        // set default email address (SIPXPBXUSER@localhost)
        server.setContacts(createDefaultContacts());
        return server;
    }

    private AlarmServerContacts createDefaultContacts() {
        AlarmServerContacts contacts = new AlarmServerContacts();
        List<String> address = new ArrayList<String>();
        address.add(getSipxUser() + DEFAULT_HOST);
        contacts.setAddresses(address);

        return contacts;
    }

    private void saveAlarmServer(AlarmServer server) {
        HibernateTemplate template = getHibernateTemplate();
        template.saveOrUpdate(server);
        template.flush();
    }

    public void replicateAlarmServer(SipxReplicationContext replicationContext, Location location) {
        AlarmServer alarmServer = getAlarmServer();
        m_alarmServerConfiguration.generate(alarmServer, getLogDirectory(), getHost());

        if (location == null) {
            replicationContext.replicate(m_alarmServerConfiguration);
        } else {
            replicationContext.replicate(location, m_alarmServerConfiguration);
        }
    }

    private void replicateAlarmsConfiguration(List<Alarm> alarms) {
        m_alarmsConfiguration.generate(alarms);
        m_replicationContext.replicate(m_alarmsConfiguration);
    }

    public List<Alarm> getAlarmTypes(String configPath, String stringPath) {
        List<Alarm> alarms = parseAlarmsConfig(configPath);
        Collections.sort(alarms);
        List<Alarm> alarmsString = parseAlarmsConfig(stringPath);
        Collections.sort(alarmsString);
        for (int i = 0; i < alarms.size(); i++) {
            Alarm alarm = alarms.get(i);
            alarm.setShortTitle(alarmsString.get(i).getShortTitle());
        }
        return alarms;
    }

    private List<Alarm> parseAlarmsConfig(String pathFile) {
        List<Alarm> alarms = new ArrayList<Alarm>();
        try {
            DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder docBuilder = docFactory.newDocumentBuilder();
            Document doc = docBuilder.parse(pathFile);
            doc.getDocumentElement().normalize();

            NodeList alarmsList = doc.getElementsByTagName("alarm");
            for (int i = 0; i < alarmsList.getLength(); i++) {
                Alarm alarmBean = new Alarm();
                Node nodeAlarm = alarmsList.item(i);
                if (nodeAlarm.getNodeType() == Node.ELEMENT_NODE) {
                    NamedNodeMap alarmMap = nodeAlarm.getAttributes();
                    Node alarmId = alarmMap.getNamedItem("id");
                    alarmBean.setAlarmId(alarmId.getNodeValue());
                    alarmBean.setCode(getNodeValue(nodeAlarm, "code"));
                    alarmBean.setSeverity(getNodeValue(nodeAlarm, "severity"));
                    alarmBean.setComponent(getNodeValue(nodeAlarm, "component"));
                    alarmBean.setShortTitle(getNodeValue(nodeAlarm, "shorttitle"));
                    alarmBean.setEmailEnabled(Boolean.parseBoolean(getAtributeValue(nodeAlarm, "action", "email")));
                    String threshold = getAtributeValue(nodeAlarm, "filter", "min_threshold");
                    int minThreshold = (threshold.equals(EMPTY) ? 0 : Integer.parseInt(threshold));
                    alarmBean.setMinThreshold(minThreshold);
                }
                alarms.add(alarmBean);
            }
        } catch (ParserConfigurationException ex) {
            LOG.error(ex);
        } catch (SAXException ex) {
            LOG.error(ex);
        } catch (IOException ex) {
            LOG.error(ex);
        }

        return alarms;
    }

    private String getNodeValue(Node nodeAlarm, String childNodeName) {
        NodeList nodeList = ((Element) nodeAlarm).getElementsByTagName(childNodeName);
        Element firstElement = (Element) nodeList.item(0);
        if (firstElement != null) {
            NodeList textFNList = firstElement.getChildNodes();
            return textFNList.item(0).getNodeValue();
        }
        return EMPTY;
    }

    private String getAtributeValue(Node nodeAlarm, String childNodeName, String atributeName) {
        NodeList nodeList = ((Element) nodeAlarm).getElementsByTagName(childNodeName);
        Element firstElement = (Element) nodeList.item(0);
        if (firstElement != null) {
            return firstElement.getAttribute(atributeName);
        }
        return EMPTY;
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
            return httpget.getResponseBodyAsStream();
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

        int last = first + pageSize;
        if (last > contents.size()) {
            last = first + contents.size() % pageSize;
        }

        List<AlarmEvent> pageContents = contents.subList(first, last);

        return pageContents;
    }

    private String getHost() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
    }

    private AlarmApi getAlarmApi() {
        return m_alarmApiProvider.getApi(m_locationsManager.getPrimaryLocation().getProcessMonitorUrl());
    }
}
