/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.proxy;

import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.cdr.CdrManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.tls.TlsPeer;
import org.sipfoundry.sipxconfig.tls.TlsPeerManager;
import org.springframework.beans.factory.annotation.Required;

public class ProxyConfiguration implements ConfigProvider {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/peeridentities-00-00";
    private static final String PROXY = "sipxproxy";
    private static final String PROXY_CFDAT = "sipxproxy.cfdat";
    private TlsPeerManager m_tlsPeerManager;
    private ProxyManager m_proxyManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(ProxyManager.FEATURE, TlsPeerManager.FEATURE)) {
            return;
        }

        FeatureManager fm = manager.getFeatureManager();
        Set<Location> locations = request.locations(manager);
        boolean isCdrOn = manager.getFeatureManager().isFeatureEnabled(CdrManager.FEATURE);
        ProxySettings settings = m_proxyManager.getSettings();
        Domain domain = manager.getDomainManager().getDomain();
        Collection<TlsPeer> peers = m_tlsPeerManager.getTlsPeers();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = fm.isFeatureEnabled(ProxyManager.FEATURE, location);
            if (!enabled) {
                ConfigUtils.enableCfengineClass(dir, PROXY_CFDAT, enabled, PROXY);
            } else {
                ConfigUtils.enableCfengineClass(dir, PROXY_CFDAT, enabled, "postgres", PROXY);
            }

            // always generate only because sipxbridge needs file and harmless to generate
            // even if not every machine needs it.
            Writer peersConfig = new FileWriter(new File(dir, "peeridentities.xml"));
            try {
                XmlFile config = new XmlFile(peersConfig);
                config.write(getDocument(peers));
            } finally {
                IOUtils.closeQuietly(peersConfig);
            }

            if (!enabled) {
                continue;
            }
            Writer proxy = new FileWriter(new File(dir, "sipXproxy-config.part"));
            try {
                write(proxy, settings, location, domain, isCdrOn);
            } finally {
                IOUtils.closeQuietly(proxy);
            }
        }
    }

    void write(Writer wtr, ProxySettings settings, Location location, Domain domain, boolean isCdrOn)
        throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(wtr);
        Setting root = settings.getSettings();
        config.writeSettings(root.getSetting("proxy-configuration"));
        config.writeSettings("SIPX_PROXY.205_subscriptionauth.", root.getSetting("subscriptionauth"));
        config.writeSettings("SIPX_PROXY.350_calleralertinfo.", root.getSetting("alert-info"));
        config.writeSettings("SIPX_PROXY.400_authrules.", root.getSetting("authrules"));
        String xferPlugin = "SIPX_PROXY.200_xfer.";
        config.writeSetting(xferPlugin, root.getSetting("msftxchghack/EXCHANGE_SERVER_FQDN"));
        config.writeSetting(xferPlugin, root.getSetting("msftxchghack/ADDITIONAL_EXCHANGE_SERVER_FQDN"));
        config.writeSetting("SIPX_PROXY.210_msftxchghack.", root.getSetting("msftxchghack/USERAGENT"));
        config.writeSettings(root.getSetting("call-rate-limit"));
        config.write("SIPX_PROXY_HOST_NAME", location.getFqdn());
        int port = settings.getSipTcpPort();
        String aliases = format("%s:%d %s:%d", location.getAddress(), port, location.getFqdn(), port);
        config.write("SIPX_PROXY_HOST_ALIASES", aliases);
        config.write("SIPX_PROXY_CALL_STATE_DB", isCdrOn ? "ENABLE" : "DISABLE");
        config.write("SIPX_PROXY_HOSTPORT", location.getAddress() + ':' + port);
        config.write("SIPX_PROXY_AUTHENTICATE_REALM", domain.getSipRealm());
    }

    public Document getDocument(Collection<TlsPeer> peers) {
        Document document = XmlFile.FACTORY.createDocument();
        final Element peerIdentities = document.addElement("peeridentities", NAMESPACE);
        for (TlsPeer peer : peers) {
            Element peerElement = peerIdentities.addElement("peer");
            peerElement.addElement("trusteddomain").setText(peer.getName());
            peerElement.addElement("internaluser").setText(peer.getInternalUser().getUserName());
        }

        return document;
    }

    @Required
    public void setTlsPeerManager(TlsPeerManager peerManager) {
        m_tlsPeerManager = peerManager;
    }

    public void setProxyManager(ProxyManager proxyManager) {
        m_proxyManager = proxyManager;
    }
}
