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
import java.util.Collection;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.springframework.beans.factory.annotation.Required;

public class ImBotConfiguration implements ConfigProvider {
    private ImBot m_imbot;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(ImBot.FEATURE, LocalizationContext.FEATURE)) {
            FeatureManager featureManager = manager.getFeatureManager();
            Collection<Location> locations = featureManager.getLocationsForEnabledFeature(ImBot.FEATURE);
            Address ivrApi = manager.getAddressManager().getSingleAddress(Ivr.REST_API);
            Address adminApi = manager.getAddressManager().getSingleAddress(AdminContext.HTTPS_ADDRESS);
            Address restApi = manager.getAddressManager().getSingleAddress(RestServer.HTTPS_API);
            for (Location location : locations) {
                File f = new File(manager.getLocationDataDirectory(location), "sipximbot.properties.cfdat");
                FileWriter wtr = new FileWriter(f);
                KeyValueConfiguration config = new KeyValueConfiguration(wtr, "=");
                ImBotSettings settings = m_imbot.getSettings();
                config.write(settings.getSettings());
                DomainManager dm = manager.getDomainManager();
                config.write("imbot.operatorAddr", "sip:operator@" + dm.getDomainName());
                config.write("imbot.sipxchangeDomainName", dm.getDomainName());
                config.write("imbot.realm", dm.getAuthorizationRealm());
                config.write("imbot.voicemailRootUrl", ivrApi.toString());
                config.write("imbot.configUrl", adminApi.toString());
                config.write("imbot.3pccSecureUrl", restApi.toString());
                config.write("imbot.callHistoryUrl", restApi.toString() + "/cdr/");

                config.write("imbot.paUserName", settings.getPersonalAssistantImId() + '@' + dm.getDomainName());
                config.write("imbot.paPassword", settings.getPersonalAssistantImPassword());

                if (featureManager.isFeatureEnabled(ImManager.FEATURE)) {
                    Address imApi = manager.getAddressManager().getSingleAddress(ImManager.XMLRPC_ADDRESS);
                    config.write("imbot.openfireHost", imApi.getAddress());
                    config.write("imbot.openfireXmlRpcPort", imApi.getPort());
                }
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    @Required
    public void setImbot(ImBot imbot) {
        m_imbot = imbot;
    }
}
