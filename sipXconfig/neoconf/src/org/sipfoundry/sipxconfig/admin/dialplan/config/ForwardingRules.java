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
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxStatusService;

/**
 * Controls very initial SIP message routing from proxy based on SIP method and potentialy message
 * content.
 */
public class ForwardingRules extends XmlFile {
    private VelocityEngine m_velocityEngine;
    private SbcManager m_sbcManager;

    private String m_template = "commserver/forwardingrules.vm";
    private Document m_doc;
    private List<String> m_routes;
    private SipxServiceManager m_sipxServiceManager;
    private VelocityContext m_velocityContext;

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }

    public void begin() {
        m_routes = new ArrayList<String>();
    }

    public void generate(IDialingRule rule) {
        m_routes.addAll(Arrays.asList(rule.getHostPatterns()));
    }

    public void end() {
        m_velocityContext = new VelocityContext();
        m_velocityContext.put("routes", m_routes);

        DefaultSbc sbc = m_sbcManager.loadDefaultSbc();
        m_velocityContext.put("sbc", sbc);

        if (sbc != null) {
            m_velocityContext.put("exportLocalIpAddress", !sbc.getRoutes().isEmpty());
        }

        m_velocityContext.put("auxSbcs", m_sbcManager.loadAuxSbcs());
        m_velocityContext.put("dollar", "$");

        // set required sipx services in context
        m_velocityContext.put("proxyService", m_sipxServiceManager
                .getServiceByBeanId(SipxProxyService.BEAN_ID));
        m_velocityContext.put("statusService", m_sipxServiceManager
                .getServiceByBeanId(SipxStatusService.BEAN_ID));
        m_velocityContext.put("registrarService", m_sipxServiceManager
                .getServiceByBeanId(SipxRegistrarService.BEAN_ID));
    }

    @Override
    protected void localizeDocument(Location location) {
        m_velocityContext.put("location", location);

        try {
            Writer out = new StringWriter();
            m_velocityEngine.mergeTemplate(m_template, "UTF-8", m_velocityContext, out);
            String xml = out.toString();
            SAXReader xmlReader = new SAXReader();
            m_doc = xmlReader.read(new StringReader(xml));
        } catch (DocumentException e) {
            throw new RuntimeException(e);
        } catch (Exception e) {
            // merge template throws exception!
            throw new RuntimeException("Error using velocity template " + m_template, e);
        }
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

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }
}
