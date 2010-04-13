/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.List;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.QName;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.SAXReader;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.CallTag;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.service.SipxPageService;
import org.sipfoundry.sipxconfig.service.SipxParkService;
import org.sipfoundry.sipxconfig.service.SipxRlsService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

/**
 * Special type of mappingrules document with a single host match matching standard SIPx hosts
 */
public class MappingRules extends RulesXmlFile {
    private static final String ORBIT_SERVER_SIP_SRV_OR_HOSTPORT = "${ORBIT_SERVER_SIP_SRV_OR_HOSTPORT}";
    private static final String RLS_SIP_SRV_OR_HOSTPORT = "${RLS_SIP_SRV_OR_HOSTPORT}";
    private static final String MY_HOSTNAME = "${MY_HOSTNAME}";
    private static final String MY_FULL_HOSTNAME = "${MY_FULL_HOSTNAME}";
    private static final String MY_IP_ADDR = "${MY_IP_ADDR}";
    private static final String SIPXCHANGE_DOMAIN_NAME = "${SIPXCHANGE_DOMAIN_NAME}";
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00";
    private static final String HOST_PATTERN = "hostPattern";

    private Document m_doc;
    private Element m_hostMatch;
    private final String m_namespace;
    private Element m_mappings;

    private SipxServiceManager m_sipxServiceManager;

    public MappingRules() {
        this(NAMESPACE);
    }

    protected MappingRules(String namespace) {
        m_namespace = namespace;
    }

    @Override
    public void begin() {
        m_doc = FACTORY.createDocument();
        QName mappingsName = FACTORY.createQName("mappings", m_namespace);
        m_mappings = m_doc.addElement(mappingsName);
        addExternalRules(m_mappings);
        m_hostMatch = addHostPatterns(getHostStrings(), getDomainManager().getDomain().getName());
    }

    Element getFirstHostMatch() {
        return m_hostMatch;
    }

    @Override
    public Document getDocument() {
        return m_doc;
    }

    @Override
    public void generate(IDialingRule rule) {
        if (rule.isInternal()) {
            generateRule(rule);
        }
    }

    protected final void generateRule(IDialingRule rule) {
        String[] hostPatterns = rule.getHostPatterns();
        Element hostMatch;
        if (hostPatterns.length > 0) {
            hostMatch = addHostPatterns(hostPatterns, null);
            m_mappings.add(hostMatch);
        } else {
            hostMatch = getFirstHostMatch();
        }

        generateRule(rule, hostMatch);
    }

    protected final void generateRule(IDialingRule rule, Element hostMatch) {
        Element userMatch = hostMatch.addElement("userMatch");
        addRuleNameComment(userMatch, rule);
        addRuleDescription(userMatch, rule);
        CallTag callTag = (CallTag) ObjectUtils.defaultIfNull(rule.getCallTag(), CallTag.UNK);
        userMatch.addElement("callTag").setText(callTag.toString());
        String[] patterns = rule.getPatterns();
        for (int i = 0; i < patterns.length; i++) {
            String pattern = patterns[i];
            Element userPattern = userMatch.addElement("userPattern");
            userPattern.setText(pattern);
        }
        addTransforms(rule, userMatch);
    }

    protected void addTransforms(IDialingRule rule, Element userMatch) {
        Element permissionMatch = userMatch.addElement("permissionMatch");
        if (rule.isTargetPermission()) {
            List<String> permissions = rule.getPermissionNames();
            for (String permission : permissions) {
                Element permissionElement = permissionMatch.addElement("permission");
                permissionElement.setText(permission);
            }
        }
        Transform[] transforms = rule.getTransforms();
        for (int i = 0; i < transforms.length; i++) {
            Transform transform = transforms[i];
            transform.addToParent(permissionMatch);
        }
    }

    protected Element addHostPatterns(String[] hostPatterns, String domainName) {
        Element hostMatch = FACTORY.createElement("hostMatch", m_namespace);

        if (StringUtils.isNotEmpty(domainName)) {
            Element domainElement = hostMatch.addElement(HOST_PATTERN);
            domainElement.setText(getDomainManager().getDomain().getName());
        }

        for (String hostPattern : hostPatterns) {
            Element pattern = hostMatch.addElement(HOST_PATTERN);
            pattern.setText(hostPattern);
        }

        return hostMatch;
    }

    @Override
    public void end() {
        m_mappings.add(m_hostMatch);
    }

    public static String[] getHostStrings() {
        return new String[] {
            MY_FULL_HOSTNAME, MY_HOSTNAME, MY_IP_ADDR
        };
    }

    @Override
    protected void localizeDocument(Location location) {
        if (location == null) {
            return;
        }

        try {
            Document document = getDocument();
            OutputFormat format = new OutputFormat();
            StringWriter stringWriter = new StringWriter();
            XMLWriter xmlWriter = new XMLWriter(stringWriter, format);
            xmlWriter.write(document);

            String rulesString = stringWriter.toString();
            rulesString = rulesString.replace(SIPXCHANGE_DOMAIN_NAME, getDomainManager().getDomain().getName());
            rulesString = rulesString.replace(MY_IP_ADDR, location.getAddress());
            rulesString = rulesString.replace(MY_FULL_HOSTNAME, location.getFqdn());
            rulesString = rulesString.replace(MY_HOSTNAME, location.getHostname());

            SipxRlsService rlsService = (SipxRlsService) m_sipxServiceManager
                    .getServiceByBeanId(SipxRlsService.BEAN_ID);
            String rlsServerSipSrvOrHostport = rlsService.getAddress() + ':' + rlsService.getRlsPort();
            rulesString = rulesString.replace(RLS_SIP_SRV_OR_HOSTPORT, rlsServerSipSrvOrHostport);

            SipxParkService parkService = (SipxParkService) m_sipxServiceManager
                    .getServiceByBeanId(SipxParkService.BEAN_ID);
            String parkServerSipSrvOrHostport = parkService.getAddress() + ':' + parkService.getParkServerSipPort();
            rulesString = rulesString.replace(ORBIT_SERVER_SIP_SRV_OR_HOSTPORT, parkServerSipSrvOrHostport);

            SipxPageService pageService = (SipxPageService) m_sipxServiceManager
                    .getServiceByBeanId(SipxPageService.BEAN_ID);
            rulesString = rulesString.replace("${PAGE_SERVER_ADDR}", pageService.getAddress());
            rulesString = rulesString.replace("${PAGE_SERVER_SIP_PORT}", pageService.getSipPort());

            StringReader stringReader = new StringReader(rulesString);
            SAXReader saxReader = new SAXReader();
            m_doc = saxReader.read(stringReader);
        } catch (IOException ioe) {
            throw new RuntimeException(ioe);
        } catch (DocumentException de) {
            throw new RuntimeException(de);
        }
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }
}
