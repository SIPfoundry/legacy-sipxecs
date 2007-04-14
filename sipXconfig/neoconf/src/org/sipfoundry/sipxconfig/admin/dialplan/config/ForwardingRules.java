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

import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.io.SAXReader;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * Controls very initial SIP message routing from proxy based on SIP method and potentialy message
 * content.
 */
public class ForwardingRules extends XmlFile {
    private VelocityEngine m_velocityEngine;
    private String m_template = "commserver/forwardingrules.vm";
    private Document m_doc;
    private List<String> m_routes;

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void begin() {
        m_routes = new ArrayList();
    }

    public void generate(IDialingRule rule) {
        m_routes.addAll(Arrays.asList(rule.getHostPatterns()));
    }

    public void end() {
        VelocityContext velocityContext = new VelocityContext();
        velocityContext.put("routes", m_routes);
        velocityContext.put("dollar", "$");
        Writer out = new StringWriter();
        try {
            m_velocityEngine.mergeTemplate(m_template, velocityContext, out);
        } catch (Exception e) {
            throw new RuntimeException("Error using velocity template " + m_template, e);
        }
        String xml = out.toString();
        SAXReader xmlReader = new SAXReader();
        try {
            m_doc = xmlReader.read(new StringReader(xml));
        } catch (DocumentException e) {
            throw new RuntimeException(e);
        }
    }

    public ConfigFileType getType() {
        return ConfigFileType.FORWARDING_RULES;
    }

    @Override
    public Document getDocument() {
        return m_doc;
    }

    public String getTemplate() {
        return m_template;
    }

    public void setTemplate(String template) {
        m_template = template;
    }
}
