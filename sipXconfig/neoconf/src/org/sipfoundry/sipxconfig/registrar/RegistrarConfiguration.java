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
package org.sipfoundry.sipxconfig.registrar;

import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Map;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.setting.PatternSettingFilter;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

public class RegistrarConfiguration implements ConfigProvider, ApplicationContextAware {
    private static final SettingFilter NO_UNDERSCORE = new PatternSettingFilter(".*/_.*");
    private static final String WEIGHT_40 = "40";
    private static final String WEIGHT_100 = "100";
    private Registrar m_registrar;
    private ApplicationContext m_context;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Registrar.FEATURE, ParkOrbitContext.FEATURE, ProxyManager.FEATURE, ImManager.FEATURE)) {
            return;
        }

        FeatureManager fm = manager.getFeatureManager();
        Set<Location> locations = request.locations(manager);
        RegistrarSettings settings = m_registrar.getSettings();
        Domain domain = manager.getDomainManager().getDomain();
        Address imApi = manager.getAddressManager().getSingleAddress(ImManager.XMLRPC_ADDRESS);
        Address presenceApi = manager.getAddressManager().getSingleAddress(Registrar.PRESENCE_MONITOR_ADDRESS);
        Address proxy = manager.getAddressManager().getSingleAddress(ProxyManager.TCP_ADDRESS);
        Address park = manager.getAddressManager().getSingleAddress(ParkOrbitContext.SIP_TCP_PORT);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = fm.isFeatureEnabled(Registrar.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxregistrar.cfdat", enabled, "sipxregistrar");
            if (enabled) {
                Writer w = new FileWriter(new File(dir, "registrar-config.part"));
                try {
                    write(w, settings, domain, location, proxy, imApi, presenceApi, park, fm);
                } finally {
                    IOUtils.closeQuietly(w);
                }
            }
        }
    }

    void write(Writer wtr, RegistrarSettings settings, Domain domain, Location location, Address proxy,
            Address imApi, Address presenceApi, Address park, FeatureManager fm) throws IOException {
        KeyValueConfiguration file = KeyValueConfiguration.colonSeparated(wtr);
        Setting root = settings.getSettings();
        file.writeSettings(SettingUtil.filter(NO_UNDERSCORE, root.getSetting("registrar-config")));
        file.writeSettings(SettingUtil.filter(NO_UNDERSCORE, root.getSetting("other")));
        // add authority levels
        file.write("SIP_REDIRECT_AUTHORITY_LEVEL.090-USERPARAM", "60");
        file.write("SIP_REDIRECT_AUTHORITY_LEVEL.100-PICKUP", "50");
        if (settings.isEarlyAliasResolutionEnabled()) {
            file.write("SIP_REDIRECT_AUTHORITY_LEVEL.110-ALIAS", WEIGHT_40);
            file.write("SIP_REDIRECT_AUTHORITY_LEVEL.120-REG", WEIGHT_40);
        } else {
            file.write("SIP_REDIRECT_AUTHORITY_LEVEL.110-REG", WEIGHT_40);
            file.write("SIP_REDIRECT_AUTHORITY_LEVEL.120-ALIAS", WEIGHT_40);
        }
        file.write("SIP_REDIRECT_AUTHORITY_LEVEL.130-MAPPING", WEIGHT_40);
        file.write("SIP_REDIRECT_AUTHORITY_LEVEL.140-FALLBACK", "30");
        file.write("SIP_REDIRECT_AUTHORITY_LEVEL.150-ISN", WEIGHT_40);
        file.write("SIP_REDIRECT_AUTHORITY_LEVEL.160-ENUM", WEIGHT_40);
        file.write("SIP_REDIRECT_AUTHORITY_LEVEL.900-PRESENCE", WEIGHT_100);
        file.write("SIP_REDIRECT_AUTHORITY_LEVEL.997-SUBSCRIBE", WEIGHT_100);
        file.write("SIP_REDIRECT_AUTHORITY_LEVEL.998-TIMEOFDAY", WEIGHT_100);
        file.write("SIP_REDIRECT_AUTHORITY_LEVEL.999-AUTHROUTER", WEIGHT_100);

        file.write("SIP_REDIRECT.100-PICKUP.ORBIT_FILENAME", "$(sipx.SIPX_CONFDIR)/orbits.xml");
        file.write("SIP_REDIRECT.130-MAPPING.MAPPING_RULES_FILENAME", "$(sipx.SIPX_CONFDIR)/mappingrules.xml");
        file.write("SIP_REDIRECT.140-FALLBACK.MAPPING_RULES_FILENAME", "$(sipx.SIPX_CONFDIR)/fallbackrules.xml");
        file.write("SIP_REDIRECT_HOOK_LIBRARY.090-USERPARAM", "$(sipx.SIPX_LIBDIR)/libRedirectorUserParam.so");
        file.write("SIP_REDIRECT_HOOK_LIBRARY.100-PICKUP", "$(sipx.SIPX_LIBDIR)/libRedirectorPickUp.so");
        String aliasRedirector = "$(sipx.SIPX_LIBDIR)/libRedirectorAliasDB.so";
        String regRedirector = "$(sipx.SIPX_LIBDIR)/libRedirectorRegDB.so";
        if (settings.isEarlyAliasResolutionEnabled()) {
            file.write("SIP_REDIRECT_HOOK_LIBRARY.110-ALIAS", aliasRedirector);
            file.write("SIP_REDIRECT_HOOK_LIBRARY.120-REG", regRedirector);
        } else {
            file.write("SIP_REDIRECT_HOOK_LIBRARY.110-REG", regRedirector);
            file.write("SIP_REDIRECT_HOOK_LIBRARY.120-ALIAS", aliasRedirector);
        }
        file.write("SIP_REDIRECT_HOOK_LIBRARY.130-MAPPING", "$(sipx.SIPX_LIBDIR)/libRedirectorMapping.so");
        file.write("SIP_REDIRECT_HOOK_LIBRARY.140-FALLBACK", "$(sipx.SIPX_LIBDIR)/libRedirectorFallback.so");
        file.write("SIP_REDIRECT_HOOK_LIBRARY.150-ISN", "$(sipx.SIPX_LIBDIR)/libRedirectorISN.so");
        file.write("SIP_REDIRECT_HOOK_LIBRARY.160-ENUM", "$(sipx.SIPX_LIBDIR)/libRedirectorENUM.so");
        file.write("SIP_REDIRECT_HOOK_LIBRARY.997-SUBSCRIBE", "$(sipx.SIPX_LIBDIR)/libRedirectorSubscribe.so");
        file.write("SIP_REDIRECT_HOOK_LIBRARY.998-TIMEOFDAY", "$(sipx.SIPX_LIBDIR)/libRedirectorTimeOfDay.so");
        file.write("SIP_REDIRECT_HOOK_LIBRARY.999-AUTHROUTER", "$(sipx.SIPX_LIBDIR)/libRedirectorAuthRouter.so");
        file.write("SIP_REGISTRAR_HOOK_LIBRARY.MWI", "$(sipx.SIPX_LIBDIR)/libRegistrarImpliedMWI.so");
        file.write("SIP_REGISTRAR_AUTHENTICATE_REALM", domain.getSipRealm());
        file.write("SIP_REGISTRAR_DOMAIN_NAME", domain.getName());
        file.write("SIP_REGISTRAR_PROXY_PORT", proxy.getPort());
        file.write("SIP_REGISTRAR_NAME", location.getFqdn());
        file.write("SIP_REGISTRAR_SYNC_WITH", "obsolete");
        file.writeSettings(root.getSetting("userparam"));
        file.writeSettings(root.getSetting("call-pick-up"));

        if (park != null) {
            String parkUri = format("%s;transport=tcp?Route=sip:%s:%d", domain.getName(), park.getAddress(),
                    park.getPort());
            file.write("SIP_REDIRECT.100-PICKUP.PARK_SERVER", parkUri);
        }

        file.writeSettings("SIP_REDIRECT.130-MAPPING.", root.getSetting("mapping"));
        file.writeSettings(root.getSetting("isn"));
        file.writeSettings(root.getSetting("enum"));

        if (imApi != null) {
            String openfireUrl = format("http://%s:%d/plugins/sipx-openfire-presence/status", imApi.getAddress(),
                    imApi.getPort());
            file.write("SIP_REDIRECT.900-PRESENCE.OPENFIRE_PRESENCE_SERVER_URL", openfireUrl);
            file.write("SIP_REDIRECT.900-PRESENCE.LOCAL_PRESENCE_MONITOR_SERVER_URL", presenceApi.toString());
            file.write("SIP_REDIRECT.900-PRESENCE.REALM", domain.getSipRealm());
            file.write("SIP_REDIRECT.900-PRESENCE.SIP_DOMAIN", domain.getName());
        }
        file.write("SIP_REDIRECT.999-AUTHROUTER.SIPX_PROXY", domain.getName() + ";transport=tcp");

        // add entries configurable in registrar plugins
        Map<String, RegistrarConfigurationPlugin> beans = m_context.getBeansOfType(RegistrarConfigurationPlugin.class);
        if (beans != null) {
            for (RegistrarConfigurationPlugin bean : beans.values()) {
                String featureId = bean.getFeatureId();
                if (featureId == null  || (fm.isFeatureEnabled(new LocationFeature(featureId))
                        || fm.isFeatureEnabled(new GlobalFeature(featureId)))) {
                    for (Map.Entry<String, String> plugin : bean.getRegistrarPlugins().entrySet()) {
                        file.write(plugin.getKey(), plugin.getValue());
                    }
                }
            }
        }
    }

    @Required
    public void setRegistrar(Registrar registrar) {
        m_registrar = registrar;
    }

    @Override
    public void setApplicationContext(ApplicationContext context) {
        m_context = context;
    }
}
