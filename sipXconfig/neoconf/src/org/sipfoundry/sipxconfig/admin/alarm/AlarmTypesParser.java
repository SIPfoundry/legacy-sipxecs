/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.alarm;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public class AlarmTypesParser {
    private static final Log LOG = LogFactory.getLog(AlarmTypesParser.class);

    private HibernateTemplate m_hibernate;

    // Constructor
    public AlarmTypesParser(HibernateTemplate hibernate) {
        m_hibernate = hibernate;
    }

    public List<Alarm> getTypes(InputStream isAlarmsConfig, InputStream isAlarmsString) {
        List<Alarm> alarms = parseAlarmsConfig(isAlarmsConfig);
        Collections.sort(alarms);
        List<Alarm> alarmsString = parseAlarmsConfig(isAlarmsString);
        Collections.sort(alarmsString);
        for (int i = 0; i < alarms.size(); i++) {
            Alarm alarm = alarms.get(i);
            alarm.setShortTitle(alarmsString.get(i).getShortTitle());
        }
        return alarms;
    }

    private Alarm loadAlarm(String alarmIdValue) {
        List<Alarm> alarms = m_hibernate.findByNamedQueryAndNamedParam("alarmForAlarmId", "alarmId", alarmIdValue);
        if (alarms.size() > 0) {
            return alarms.get(0);
        }
        return null;
    }

    private List<Alarm> parseAlarmsConfig(InputStream is) {
        List<Alarm> alarms = new ArrayList<Alarm>();
        try {
            DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder docBuilder = docFactory.newDocumentBuilder();
            Document doc = docBuilder.parse(is);
            doc.getDocumentElement().normalize();

            NodeList alarmsList = doc.getElementsByTagName("alarm");
            for (int i = 0; i < alarmsList.getLength(); i++) {
                Alarm alarmBean = new Alarm();
                Node nodeAlarm = alarmsList.item(i);
                if (nodeAlarm.getNodeType() == Node.ELEMENT_NODE) {
                    NamedNodeMap alarmMap = nodeAlarm.getAttributes();
                    Node alarmId = alarmMap.getNamedItem("id");
                    String alarmIdValue = alarmId.getNodeValue();
                    Alarm storedAlarm = loadAlarm(alarmIdValue);
                    alarmBean.setAlarmId(alarmIdValue);
                    alarmBean.setCode(getNodeValue(nodeAlarm, "code"));
                    alarmBean.setSeverity(getNodeValue(nodeAlarm, "severity"));
                    alarmBean.setComponent(getNodeValue(nodeAlarm, "component"));
                    alarmBean.setShortTitle(getNodeValue(nodeAlarm, "shorttitle"));
                    if (storedAlarm != null) {
                        alarmBean.setGroupName(storedAlarm.getGroupName());
                        alarmBean.setMinThreshold(storedAlarm.getMinThreshold());
                    } else {
                        alarmBean.setGroupName(getAtributeValue(nodeAlarm, "action", "email"));
                        String threshold = getAtributeValue(nodeAlarm, "filter", "min_threshold");
                        int minThreshold = (StringUtils.isBlank(threshold) ? 0 : Integer.parseInt(threshold));
                        alarmBean.setMinThreshold(minThreshold);
                    }
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
        return StringUtils.EMPTY;
    }

    private String getAtributeValue(Node nodeAlarm, String childNodeName, String atributeName) {
        NodeList nodeList = ((Element) nodeAlarm).getElementsByTagName(childNodeName);
        Element firstElement = (Element) nodeList.item(0);
        if (firstElement != null) {
            return firstElement.getAttribute(atributeName);
        }
        return StringUtils.EMPTY;
    }
}
