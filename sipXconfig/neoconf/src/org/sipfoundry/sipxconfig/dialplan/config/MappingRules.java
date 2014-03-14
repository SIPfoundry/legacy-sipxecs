/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan.config;

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
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.CallTag;
import org.sipfoundry.sipxconfig.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;

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
    private AddressManager m_addressManager;

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
        m_hostMatch = addHostPatterns(getHostStrings(), getDomainName());
    }

    Element getFirstHostMatch() {
        return m_hostMatch;
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
            domainElement.setText(getDomainName());
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

    Document getPreLocalizedDocument() {
        return m_doc;
    }

    @Override
    protected Document getDocument() {
        Location location = getLocation();
        if (location == null) {
            return m_doc;
        }

        try {
            OutputFormat format = new OutputFormat();
            StringWriter stringWriter = new StringWriter();
            XMLWriter xmlWriter = new XMLWriter(stringWriter, format);
            xmlWriter.write(m_doc);

            String rulesString = stringWriter.toString();
            rulesString = rulesString.replace(SIPXCHANGE_DOMAIN_NAME, getDomainName());
            rulesString = rulesString.replace(MY_IP_ADDR, location.getAddress());
            rulesString = rulesString.replace(MY_FULL_HOSTNAME, location.getFqdn());
            rulesString = rulesString.replace(MY_HOSTNAME, location.getHostname());

            Address park = m_addressManager.getSingleAddress(ParkOrbitContext.SIP_TCP_PORT, location);
            if (park != null) {
                rulesString = rulesString.replace(ORBIT_SERVER_SIP_SRV_OR_HOSTPORT, park.addressColonPort());
            }

            Address page = m_addressManager.getSingleAddress(PagingContext.SIP_TCP, location);
            if (page != null) {
                rulesString = rulesString.replace("${PAGE_SERVER_ADDR}", page.getAddress());
                rulesString = rulesString.replace("${PAGE_SERVER_SIP_PORT}", String.valueOf(page.getPort()));
            }

            StringReader stringReader = new StringReader(rulesString);
            SAXReader saxReader = new SAXReader();
            return saxReader.read(stringReader);
        } catch (IOException ioe) {
            throw new RuntimeException(ioe);
        } catch (DocumentException de) {
            throw new RuntimeException(de);
        }
    }

    public AddressManager getAddressManager() {
        return m_addressManager;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }
}
