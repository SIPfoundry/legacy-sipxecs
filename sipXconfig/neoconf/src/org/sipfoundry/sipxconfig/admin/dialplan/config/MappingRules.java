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

import java.util.List;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.QName;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * Special type of mappingrules document with a single host match matching standard SIPx hosts
 */
public class MappingRules extends RulesXmlFile {
    public static final String[] HOSTS = {
        "${SIPXCHANGE_DOMAIN_NAME}", "${MY_FULL_HOSTNAME}", "${MY_HOSTNAME}", "${MY_IP_ADDR}"
    };

    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00";

    private Document m_doc;
    private Element m_hostMatch;
    private final String m_namespace;
    private Element m_mappings;

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
        m_hostMatch = addHostPatterns(HOSTS);
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
            hostMatch = addHostPatterns(hostPatterns);
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

    protected Element addHostPatterns(String[] hostPatterns) {
        Element hostMatch = FACTORY.createElement("hostMatch", m_namespace);
        for (String hostPattern : hostPatterns) {
            Element pattern = hostMatch.addElement("hostPattern");
            pattern.setText(hostPattern);
        }
        return hostMatch;
    }

    @Override
    public void end() {
        m_mappings.add(m_hostMatch);
    }
}
