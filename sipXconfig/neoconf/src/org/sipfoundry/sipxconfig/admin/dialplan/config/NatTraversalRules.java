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

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.io.SAXReader;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;

public class NatTraversalRules extends XmlFile {
    private VelocityEngine m_velocityEngine;
    private SbcManager m_sbcManager;
    private SbcDeviceManager m_sbcDeviceManager;
    private NatTraversalManager m_natTraversalManager;

    private String m_template = "nattraversal/nattraversalrules.vm";
    private Document m_doc;

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public void setNatTraversalManager(NatTraversalManager natTraversalManager) {
        m_natTraversalManager = natTraversalManager;
    }

    public void end() {
        VelocityContext velocityContext = new VelocityContext();
        DefaultSbc sbc = m_sbcManager.loadDefaultSbc();
        NatTraversal natTraversal = m_natTraversalManager.getNatTraversal();

        velocityContext.put("sbc", sbc);
        velocityContext.put("dollar", "$");
        velocityContext.put("state", natTraversal.isEnabled() ? "enabled" : "disabled");
        velocityContext.put("behindnat", natTraversal.isBehindnat());

        Object publicAddress = natTraversal.getInfoPublicAddress().getTypedValue();
        Object maxConcRelays = natTraversal.getInfoMaxConcRelays().getTypedValue();

        velocityContext.put("publicaddress", publicAddress == null ? "" : publicAddress);
        velocityContext.put("relayaggressiveness", natTraversal.getInfoAggressiveness().getTypedValue());
        velocityContext.put("maxconcurrentrelays", maxConcRelays == null ? 0 : maxConcRelays);

        BridgeSbc sbcBridge = m_sbcDeviceManager.getBridgeSbc();

        velocityContext.put("bridge", sbcBridge == null ? false : true);

        if (sbcBridge != null) {
            velocityContext.put("mediarelaypublicaddress", sbcBridge.
                    getSettingTypedValue("bridge-configuration/external-address"));
            velocityContext.put("mediarelaynativeaddress", sbcBridge.
                    getSettingTypedValue("bridge-configuration/local-address"));
            velocityContext.put("mediarelayxml-rpc-port", sbcBridge.
                    getSettingTypedValue("bridge-configuration/external-port"));
        }

        try {
            Writer out = new StringWriter();
            m_velocityEngine.mergeTemplate(m_template, "UTF-8", velocityContext, out);
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

    public ConfigFileType getType() {
        return ConfigFileType.NAT_TRAVERSAL_RULES;
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
