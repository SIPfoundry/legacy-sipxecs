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

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;
import org.sipfoundry.sipxconfig.admin.dialplan.RoutingException;
import org.sipfoundry.sipxconfig.gateway.Gateway;

/**
 * EmergencyRoutingRules generates XML markup for e911rules.xml file
 * 
 * 
 */
public class EmergencyRoutingRules extends XmlFile {
    private static final String E911RULES_XML = "e911rules.xml";
    private static final String E911RULES_NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/urle911-00-00";

    private Document m_document;

    public Document getDocument() {
        return m_document;
    }

    public void generate(EmergencyRouting er, String domainName) {
        m_document = FACTORY.createDocument();
        Element mappings = m_document.addElement("mappings", E911RULES_NAMESPACE);
        Element defaultMatch = mappings.addElement("defaultMatch");
        Gateway defaultGateway = er.getDefaultGateway();
        if (defaultGateway != null) {
            String externalNumber = er.getExternalNumber();
            String address = defaultGateway.getGatewayAddress();
            String transport = defaultGateway.getGatewayTransportUrlParam();

            generateTransform(defaultMatch, externalNumber, address, transport);
        }
        Collection exceptions = er.getExceptions();
        for (Iterator i = exceptions.iterator(); i.hasNext();) {
            RoutingException exception = (RoutingException) i.next();
            generateException(mappings, exception, domainName);
        }
    }

    /**
     * Generate markup for a single exception
     * 
     * <code>
     *  <userMatch>
     *      <userPattern>523124234</userPattern>
     *      <userPattern>623124234</userPattern>
     *      <transform>
     *          <user>911</user>
     *          <host>10.1.2.1</host>
     *          <urlparams>transport="transport"</urlparams>
     *      </transform>
     *  </userMatch>
     * </code>
     * 
     * @param mappings parent element
     * @param exception bean representing routing exception
     * @param domainName name of the domain for which exceptions are generated
     */
    private void generateException(Element mappings, RoutingException exception, String domainName) {
        Element userMatch = mappings.addElement("userMatch");
        String[] patterns = exception.getPatterns(domainName);
        for (int j = 0; j < patterns.length; j++) {
            Element userPattern = userMatch.addElement("userPattern");
            userPattern.setText(patterns[j]);
        }
        Gateway gateway = exception.getGateway();
        if (gateway != null) {
            String address = gateway.getGatewayAddress();
            String transport = gateway.getGatewayTransportUrlParam();
            String externalNumber = exception.getExternalNumber();
            generateTransform(userMatch, externalNumber, address, transport);
        }
    }

    /**
     * Generate transform element
     * 
     * <code>
     *      <transform>
     *          <user>externalNumber</user>
     *          <host>address</host>
     *          <urlparams>transport="transport"</urlparams>
     *      </transform>
     * </code>
     * 
     * @param userMatch
     * @param externalNumber
     * @param address
     */
    private void generateTransform(Element userMatch, String externalNumber, String address, String transport) {
        Element transform = userMatch.addElement("transform");
        Element user = transform.addElement("user");
        user.setText(externalNumber);
        Element host = transform.addElement("host");
        host.setText(address);
        if (transport != null) {
            Element urlParams = transform.addElement("urlparams");
            urlParams.setText(transport);
        }
    }

    /**
     * Writes to file in a specified directory
     * 
     * @param configDirectory configureation directory
     * @throws IOException
     */
    public void writeToFile(String configDirectory) throws IOException {
        File parent = new File(configDirectory);
        writeToFile(parent, E911RULES_XML);
    }

    public ConfigFileType getType() {
        return ConfigFileType.E911_RULES;
    }
}
