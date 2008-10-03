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

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.alarm.Alarm;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmConfiguration;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServer;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerActivatedEvent;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerConfiguration;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerContacts;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmsActivatedEvent;
import org.sipfoundry.sipxconfig.admin.commserver.AlarmApi;
import org.sipfoundry.sipxconfig.admin.commserver.Process;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public class AlarmContextImpl extends SipxHibernateDaoSupport implements AlarmContext {
    private static final Log LOG = LogFactory.getLog(AlarmContextImpl.class);
    private static final String DEFAULT_HOST = "@localhost";
    private static final String EMPTY = "";
    private String m_host;
    private AlarmApi m_alarmApi;
    private SipxReplicationContext m_replicationContext;
    private SipxProcessContext m_processContext;
    private AlarmServerConfiguration m_alarmServerConfiguration;
    private AlarmConfiguration m_alarmsConfiguration;
    private String m_sipxUser;
    private String m_hostName;    
    private String m_logDirectory;
    private String m_configDirectory;
    private String m_alarmsStringsDirectory;

    @Required
    public void setHost(String host) {
        m_host = host;
    }

    @Required
    public void setAlarmApi(AlarmApi alarmApi) {
        m_alarmApi = alarmApi;
    }

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
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

    public String getHostName() {
        return m_hostName;
    }

    public void setHostName(String hostName) {
        m_hostName = hostName;
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
            m_alarmApi.raiseAlarm(m_host, alarmId, alarmParams);
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }

    public void reloadAlarms() {
        try {
            m_alarmApi.reloadAlarms(m_host);
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }
    
    public void deployAlarmServer(AlarmServer server) {
        // save configuration
        saveAlarmServer(server);
        // replicate new configuration
        replicateAlarmServer();
        Process service = m_processContext.getProcess(SipxProcessModel.ProcessName.PROXY);
        m_processContext.restartOnEvent(Arrays.asList(service), AlarmServerActivatedEvent.class);
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

    public void replicateAlarmServer() {
        AlarmServer alarmServer = getAlarmServer();
        m_alarmServerConfiguration.generate(alarmServer, getLogDirectory(), getHostName());
        m_replicationContext.replicate(m_alarmServerConfiguration);
        m_replicationContext.publishEvent(new AlarmServerActivatedEvent(
                m_alarmServerConfiguration));
    }

    public void deployAlarmTypes(List<Alarm> alarms, List<Alarm> selectedAlarm) {
        for (Alarm alarm : alarms) {
            if (selectedAlarm.contains(alarm)) {
                alarm.setEmailEnabled(true);
            } else {
                alarm.setEmailEnabled(false);
            }
        }
        // write new configuration        
        replicateAlarmsConfiguration(alarms);
    }    
    
    private void replicateAlarmsConfiguration(List<Alarm> alarms) {
        m_alarmsConfiguration.generate(alarms);
        m_replicationContext.replicate(m_alarmsConfiguration);
        m_replicationContext.publishEvent(new AlarmsActivatedEvent(
                m_alarmsConfiguration));
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
                    alarmBean.setEmailEnabled(Boolean.parseBoolean(getAtributeValue(nodeAlarm,
                            "action", "email")));
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
}
