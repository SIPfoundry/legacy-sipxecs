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

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;

public class NatTraversalRules extends TemplateConfigurationFile {
    private SbcManager m_sbcManager;
    private SbcDeviceManager m_sbcDeviceManager;
    private NatTraversalManager m_natTraversalManager;

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public void setNatTraversalManager(NatTraversalManager natTraversalManager) {
        m_natTraversalManager = natTraversalManager;
    }

    protected VelocityContext setupContext() {
        VelocityContext context = super.setupContext();
        DefaultSbc sbc = m_sbcManager.loadDefaultSbc();
        NatTraversal natTraversal = m_natTraversalManager.getNatTraversal();

        context.put("sbc", sbc);
        context.put("state", natTraversal.isEnabled() ? "enabled" : "disabled");
        context.put("behindnat", natTraversal.isBehindnat());

        Object publicAddress = natTraversal.getInfoPublicAddress().getTypedValue();
        Object maxConcRelays = natTraversal.getInfoMaxConcRelays().getTypedValue();

        context.put("publicaddress", publicAddress == null ? "" : publicAddress);
        context.put("relayaggressiveness", natTraversal.getInfoAggressiveness().getTypedValue());
        context.put("maxconcurrentrelays", maxConcRelays == null ? 0 : maxConcRelays);

        BridgeSbc sbcBridge = m_sbcDeviceManager.getBridgeSbc();

        context.put("bridge", sbcBridge == null ? false : true);

        if (sbcBridge != null) {
            Object externalAddress = sbcBridge.getSettingTypedValue("bridge-configuration/external-address");
            context.put("mediarelaypublicaddress", externalAddress);
            Object localAddress = sbcBridge.getSettingTypedValue("bridge-configuration/local-address");
            context.put("mediarelaynativeaddress", localAddress);
            Object externalPort = sbcBridge.getSettingTypedValue("bridge-configuration/external-port");
            context.put("mediarelayxml-rpc-port", externalPort);
        }

        return context;
    }

    public ConfigFileType getType() {
        return ConfigFileType.NAT_TRAVERSAL_RULES;
    }
}
