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
package org.sipfoundry.sipxconfig.imbot;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.springframework.beans.factory.annotation.Required;

public class ImBotConfiguration implements ConfigProvider {
    private ImBot m_imbot;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(ImBot.FEATURE, LocalizationContext.FEATURE)) {
            return;
        }
        List<Location> restServerLocations = manager.getFeatureManager().getLocationsForEnabledFeature(
                RestServer.FEATURE);
        if (restServerLocations.isEmpty()) {
            return;
        }
        FeatureManager featureManager = manager.getFeatureManager();

        Address ivr = manager.getAddressManager().getSingleAddress(Ivr.REST_API);
        Address admin = manager.getAddressManager().getSingleAddress(AdminContext.HTTP_ADDRESS_AUTH);
        Address rest = manager.getAddressManager().getSingleAddress(RestServer.HTTP_API);
        Address imApi = manager.getAddressManager().getSingleAddress(ImManager.XMLRPC_ADDRESS);
        Domain domain = manager.getDomainManager().getDomain();
        ImBotSettings settings = m_imbot.getSettings();
        Setting imbotSettings = settings.getSettings().getSetting("imbot");
        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = featureManager.isFeatureEnabled(ImBot.FEATURE, location);

            ConfigUtils.enableCfengineClass(dir, "sipximbot.cfdat", enabled, "sipximbot");
            if (!enabled) {
                continue;
            }

            String log4jFileName = "log4j-imbot.properties.part";
            SettingUtil.writeLog4jSetting(imbotSettings, dir, log4jFileName);

            File f = new File(manager.getLocationDataDirectory(location), "sipximbot.properties.part");
            Writer wtr = new FileWriter(f);
            try {
                write(wtr, settings, domain, ivr, admin, rest, imApi);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    protected static void write(Writer wtr, ImBotSettings settings, Domain domain, Address ivr, Address admin,
        Address rest,
            Address imApi) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(wtr);
        config.write("imbot.httpport", settings.getHttpPort());
        config.write("imbot.locale", settings.getLocale());
        config.write("imbot.paUserName", settings.getPersonalAssistantImId() + '@' + domain.getName());
        config.write("imbot.paPassword", settings.getPersonalAssistantImPassword());
        config.write("imbot.voicemailRootUrl", ivr);
        config.write("imbot.configUrl", admin);
        if (rest != null) {
            config.write("imbot.3pccSecureUrl", rest);
            config.write("imbot.callHistoryUrl", rest.toString() + "/cdr/");
        }
        if (imApi != null) {
            config.write("imbot.openfireHost", domain.getNetworkName());
            config.write("imbot.openfireXmlRpcPort", imApi.getPort());
        }
    }

    @Required
    public void setImbot(ImBot imbot) {
        m_imbot = imbot;
    }
}
