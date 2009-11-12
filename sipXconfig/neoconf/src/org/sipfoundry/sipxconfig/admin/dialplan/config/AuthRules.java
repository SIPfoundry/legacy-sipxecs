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
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.QName;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.SAXReader;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.permission.PermissionName;

/**
 * Authorization rule generator.
 *
 * One dialing rules corresponds to one hostMatch element. All gateways end up in hostPatterns,
 * all dialing patterns are put in userPatterns. Permissions are added to the resulting
 * permissions match code.
 *
 * <code>
 * <hostMatch>
 *    <hostPattern>gateway addresses</hostPattern>
 *    <userMatch>
 *      <userPattern>sos</userPattern>
 *      <permissionMatch>name of the permission</permissionMatch>
 *    </userMatch>
 * </hostMatch>
 * </code>
 *
 */
public class AuthRules extends RulesXmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00";
    private static final String NO_ACCESS_RULE = "Reject all other calls to the gateways"
            + " that are not handled by the earlier rules";
    private static final String PERMISSION = "permission";
    private static final String PERMISSION_MATCH = "permissionMatch";
    private static final String USER_PATTERN = "userPattern";
    private static final String USER_MATCH = "userMatch";
    private static final String HOST_PATTERN = "hostPattern";
    private static final String HOST_MATCH = "hostMatch";

    private Document m_doc;
    private Set<Gateway> m_gateways;

    public AuthRules() {
    }

    @Override
    public void begin() {
        m_doc = FACTORY.createDocument();
        QName mappingsName = FACTORY.createQName("mappings", NAMESPACE);
        Element mappings = m_doc.addElement(mappingsName);
        addExternalRules(mappings);

        // Create a new gateways set. It will be populated as part of generate call
        m_gateways = new HashSet<Gateway>();
    }

    @Override
    public void generate(IDialingRule rule) {
        if (!rule.isAuthorizationChecked()) {
            // do not generate entries for rules that do not require authorization
            return;
        }
        List<Gateway> gateways = rule.getEnabledGateways();
        List<String> permissions = rule.getPermissionNames();
        Element mappings = m_doc.getRootElement();
        for (Gateway gateway : gateways) {
            m_gateways.add(gateway);
            String host = gateway.getGatewayAddress();
            Element hostMatch = mappings.addElement(HOST_MATCH);
            addRuleName(hostMatch, rule);
            addRuleDescription(hostMatch, rule);
            addRuleType(hostMatch, rule);
            Element hostPattern = hostMatch.addElement(HOST_PATTERN);
            hostPattern.setText(host);
            addPermissions(rule, hostMatch, permissions, gateway);
        }
        // if we have no gateways we still need to generate entries for "source" permission rules
        if (gateways.isEmpty() && !rule.isTargetPermission() && !permissions.isEmpty()) {
            Element hostMatch = mappings.addElement(HOST_MATCH);
            addRuleName(hostMatch, rule);
            addRuleDescription(hostMatch, rule);

            Element domainElement = hostMatch.addElement(HOST_PATTERN);
            domainElement.setText(getDomainManager().getDomain().getName());

            for (String host : MappingRules.getHostStrings()) {
                Element hostPattern = hostMatch.addElement(HOST_PATTERN);
                hostPattern.setText(host);
            }
            addPermissions(rule, hostMatch, permissions, null);
        }
    }

    @Override
    public Document getDocument() {
        return m_doc;
    }

    @Override
    public void end() {
        generateNoAccess(m_gateways);
    }

    private void addPermissions(IDialingRule rule, Element parent, List<String> permissions, Gateway gateway) {
        Element userMatch = parent.addElement(USER_MATCH);
        String[] patterns = rule.getTransformedPatterns(gateway);
        for (int i = 0; i < patterns.length; i++) {
            String pattern = patterns[i];
            Element userPattern = userMatch.addElement(USER_PATTERN);
            userPattern.setText(pattern);
        }
        // even if no permission is specified (permission list is empty) we create empty
        // element
        Element permissionMatch = userMatch.addElement(PERMISSION_MATCH);
        for (String permission : permissions) {
            Element pelement = permissionMatch.addElement(PERMISSION);
            pelement.setText(permission);
        }
    }

    void generateNoAccess(Collection<Gateway> gateways) {
        Element mappings = m_doc.getRootElement();
        Element hostMatch = mappings.addElement(HOST_MATCH);
        hostMatch.addElement("description").setText(NO_ACCESS_RULE);
        for (Gateway gateway : gateways) {
            Element hostPattern = hostMatch.addElement(HOST_PATTERN);
            hostPattern.setText(gateway.getGatewayAddress());
        }
        Element userMatch = hostMatch.addElement(USER_MATCH);
        userMatch.addElement(USER_PATTERN).setText(".");
        Element permissionMatch = userMatch.addElement(PERMISSION_MATCH);
        permissionMatch.addElement(PERMISSION).setText(PermissionName.NO_ACCESS.getName());
    }

    @Override
    protected void localizeDocument(Location location) {
        if (location == null) {
            return;
        }

        try {
            Document document = getDocument();
            OutputFormat format = createFormat();
            StringWriter stringWriter = new StringWriter();
            XMLWriter xmlWriter = new XMLWriter(stringWriter, format);
            xmlWriter.write(document);

            String rulesString = stringWriter.toString();
            rulesString = rulesString.replace("${SIPXCHANGE_DOMAIN_NAME}", getDomainManager().getDomain().getName());
            rulesString = rulesString.replace("${MY_IP_ADDR}", location.getAddress());
            rulesString = rulesString.replace("${MY_FULL_HOSTNAME}", location.getFqdn());
            rulesString = rulesString.replace("${MY_HOSTNAME}", location.getHostname());

            StringReader stringReader = new StringReader(rulesString);
            SAXReader saxReader = new SAXReader();
            m_doc = saxReader.read(stringReader);
        } catch (IOException ioe) {
            throw new RuntimeException(ioe);
        } catch (DocumentException de) {
            throw new RuntimeException(de);
        }
    }
}
