/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.tunnel;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.Map;
import java.util.Set;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cert.CertificateAuthorityGenerator;
import org.sipfoundry.sipxconfig.cert.CertificateManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.firewall.CustomFirewallRule;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallCustomRuleProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;

public class TunnelConfiguration implements ConfigProvider, FeatureListener, FirewallCustomRuleProvider {
    private static final String TUNNEL_CERT = "tunnelCert";
    private ConfigManager m_configManager;
    private FirewallManager m_firewallManager;
    private LocationsManager m_locationsManager;
    private AddressManager m_addressManager;
    private TunnelManager m_tunnelManager;
    private FeatureManager m_featureManager;
    private CertificateManager m_certificateManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(TunnelManager.FEATURE, FirewallManager.FEATURE)) {
            return;
        }
        Set<Location> locations = request.locations(manager);
        boolean enabled = m_featureManager.isFeatureEnabled(TunnelManager.FEATURE);

        // security
        if (enabled) {
            String certificate = m_certificateManager.getNamedCertificate(TUNNEL_CERT);
            if (certificate == null) {
                certificate = generateCertificate();
            }
            File gdir = manager.getGlobalDataDirectory();
            FileUtils.writeStringToFile(new File(gdir, "tunnel.crt"), certificate);
            String key = m_certificateManager.getNamedPrivateKey(TUNNEL_CERT);
            FileUtils.writeStringToFile(new File(gdir, "tunnel.key"), key);
        }

        // config
        TunnelSettings settings = m_tunnelManager.getSettings();
        TunnelArchitect architect = getArchitect(request.getRequestData(), settings, enabled);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            ConfigUtils.enableCfengineClass(dir, "tunnel.cfdat", enabled, "tunnel");
            if (!enabled) {
                continue;
            }
            Collection<AllowedIncomingTunnel> in = architect.getAllowedIncomingTunnels(location);
            Collection<RemoteOutgoingTunnel> out = architect.getRemoteOutgoingTunnels(location);
            Writer w = new FileWriter(new File(dir, "tunnel.yaml"));
            try {
                writeConfig(w, in, out, settings);
            } finally {
                IOUtils.closeQuietly(w);
            }
        }
    }

    String generateCertificate() {
        // With no verify configured on tunnel, no need for valid CA
        CertificateAuthorityGenerator gen = new CertificateAuthorityGenerator(TUNNEL_CERT, "self");
        String key = gen.getPrivateKeyText();
        String cert = gen.getCertificateText();
        m_certificateManager.updateNamedCertificate(TUNNEL_CERT, cert, key, null);
        return cert;
    }

    TunnelArchitect getArchitect(Map<Object, Object> requestData, TunnelSettings settings, boolean enabled) {
        TunnelArchitect architect = (TunnelArchitect) requestData.get(this);
        if (architect == null) {
            architect = new TunnelArchitect();
            if (enabled) {
                architect.setAddressManager(m_addressManager);
                architect.setServerStartPort(settings.getServerStartPort());
                architect.setClientStartPort(settings.getClientStartPort());
                architect.build(m_locationsManager.getLocationsList(), m_firewallManager.getDefaultFirewallRules());
            }
        }

        return architect;
    }

    private void writeConfig(Writer w, Collection<AllowedIncomingTunnel> in, Collection<RemoteOutgoingTunnel> out,
        TunnelSettings settings) throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);
        c.startArray("incoming");
        String name = ":name";
        String localPort = ":local_port";
        for (AllowedIncomingTunnel i : in) {
            c.write(name, i.getName());
            c.write(localPort, i.getLocalhostPort());
            c.write(":incoming_port", i.getAllowedConnectionsPort());
            c.nextElement();
        }
        c.endArray();
        c.startArray("outgoing");
        for (RemoteOutgoingTunnel o : out) {
            c.write(name, o.getName());
            c.write(localPort, o.getLocalhostPort());
            c.write(":remote_port", o.getPortOnRemoteMachine());
            c.write(":remote_address", o.getRemoteMachineAddress());
            c.nextElement();
        }
        c.endArray();
        c.writeSettings(settings.getSettings());
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setFirewallManager(FirewallManager firewallManager) {
        m_firewallManager = firewallManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public void setTunnelManager(TunnelManager tunnelManager) {
        m_tunnelManager = tunnelManager;
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return null;
    }

    @Override
    public Collection<CustomFirewallRule> getCustomRules(FirewallManager manager, Location location,
            Map<Object, Object> requestData) {
        boolean enabled = m_featureManager.isFeatureEnabled(TunnelManager.FEATURE);
        if (!enabled) {
            return null;
        }
        TunnelSettings settings = m_tunnelManager.getSettings();
        TunnelArchitect architect = getArchitect(requestData, settings, enabled);
        TunnelFirewallRules builder = new TunnelFirewallRules();
        Collection<RemoteOutgoingTunnel> out = architect.getRemoteOutgoingTunnels(location);
        Collection<CustomFirewallRule> rules = builder.build(out);
        return rules;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setCertificateManager(CertificateManager certificateManager) {
        m_certificateManager = certificateManager;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        // firewall will be configured with "transparent proxy" rules to leverage tunnels
        validator.requiresGlobalFeature(TunnelManager.FEATURE, FirewallManager.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        // every feature enable/disable will trigger firewall rules to reconfig
        // because cannot tell what this affects
        m_configManager.configureEverywhere(FirewallManager.FEATURE);
    }
}
