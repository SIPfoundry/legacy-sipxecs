/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.imbot;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.sipfoundry.sipxconfig.setting.PatternSettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.springframework.beans.factory.annotation.Required;

public class ImBotConfiguration implements ConfigProvider {
    private static final PatternSettingFilter SKIP_UNDERSCORE = new PatternSettingFilter("imbot/_.*");
    private ImBot m_imbot;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(ImBot.FEATURE, LocalizationContext.FEATURE)) {
            return;
        }

        FeatureManager featureManager = manager.getFeatureManager();
        Collection<Location> locations = featureManager.getLocationsForEnabledFeature(ImBot.FEATURE);
        if (locations.isEmpty()) {
            return;
        }

        Address ivr = manager.getAddressManager().getSingleAddress(Ivr.REST_API);
        Address admin = manager.getAddressManager().getSingleAddress(AdminContext.HTTPS_ADDRESS);
        Address rest = manager.getAddressManager().getSingleAddress(RestServer.HTTPS_API);
        Address imApi = manager.getAddressManager().getSingleAddress(ImManager.XMLRPC_ADDRESS);
        Domain domain = manager.getDomainManager().getDomain();
        ImBotSettings settings = m_imbot.getSettings();
        for (Location location : locations) {
            File f = new File(manager.getLocationDataDirectory(location), "sipximbot.properties.cfdat");
            Writer wtr = new FileWriter(f);
            try {
                write(wtr, settings, domain, ivr, admin, rest, imApi);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    void write(Writer wtr, ImBotSettings settings, Domain domain, Address ivr, Address admin, Address rest,
            Address imApi) throws IOException {
        KeyValueConfiguration config = new KeyValueConfiguration(wtr, "=");
        config.write(SettingUtil.filter(SKIP_UNDERSCORE, settings.getSettings().getSetting("imbot")));
        config.write("imbot.paUserName", settings.getPersonalAssistantImId() + '@' + domain.getName());
        config.write("imbot.sipxchangeDomainName", domain.getName());
        config.write("imbot.realm", domain.getSipRealm());
        config.write("imbot.voicemailRootUrl", ivr);
        config.write("imbot.configUrl", admin);
        config.write("imbot.3pccSecureUrl", rest);
        config.write("imbot.callHistoryUrl", rest.toString() + "/cdr/");
        if (imApi != null) {
            config.write("imbot.openfireHost", imApi.getAddress());
            config.write("imbot.openfireXmlRpcPort", imApi.getPort());
        }
    }

    @Required
    public void setImbot(ImBot imbot) {
        m_imbot = imbot;
    }
}
