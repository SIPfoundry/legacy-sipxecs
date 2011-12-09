/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.ivr;

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
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.mwi.Mwi;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.springframework.beans.factory.annotation.Required;

public class IvrConfig implements ConfigProvider {
    private Ivr m_ivr;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(DialPlanContext.FEATURE, Ivr.FEATURE)) {
            FeatureManager featureManager = manager.getFeatureManager();
            Collection<Location> locations = featureManager.getLocationsForEnabledFeature(Ivr.FEATURE);
            Address mwiApi = manager.getAddressManager().getSingleAddress(Mwi.HTTP_API);
            Address adminApi = manager.getAddressManager().getSingleAddress(AdminContext.HTTPS_ADDRESS);
            Address supervisorApi = manager.getAddressManager().getSingleAddress(ConfigManager.SUPERVISOR_ADDRESS);
            Address restApi = manager.getAddressManager().getSingleAddress(RestServer.HTTPS_API);
            for (Location location : locations) {
                File f = new File(manager.getLocationDataDirectory(location), "sipxivr.properties.cfdat");
                FileWriter wtr = new FileWriter(f);
                KeyValueConfiguration config = new KeyValueConfiguration(wtr, "=");
                IvrSettings settings = m_ivr.getSettings();
                config.write(settings.getSettings());
                DomainManager dm = manager.getDomainManager();
                config.write("ivr.operatorAddr", "sip:operator@" + dm.getDomainName());
                config.write("ivr.sipxchangeDomainName", dm.getDomainName());
                config.write("ivr.realm", dm.getAuthorizationRealm());
                config.write("ivr.mwiUrl", mwiApi.toString());
                config.write("ivr.configUrl", mwiApi.toString());
                config.write("ivr.3pccSecureUrl", restApi.toString());
                config.write("ivr.callHistoryUrl", restApi.toString() + "/cdr/");
                config.write("ivr.sipxSupervisorHost", location.getFqdn());
                config.write("ivr.sipxSupervisorXmlRpcPort", supervisorApi.getPort());
                if (featureManager.isFeatureEnabled(ImManager.FEATURE)) {
                    Address imApi = manager.getAddressManager().getSingleAddress(ImManager.XMLRPC_ADDRESS);
                    config.write("ivr.openfireHost", imApi.getAddress());
                    config.write("ivr.openfireXmlRpcPort", imApi.getPort());
                }

                if (featureManager.isFeatureEnabled(ImBot.FEATURE)) {
                    Address bot = manager.getAddressManager().getSingleAddress(ImBot.XML_RPC);
                    config.write("ivr.sendIMUrl", bot.toString());
                }
                config.write("ivr.configAddress", adminApi.getAddress());
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    @Required
    public void setIvr(Ivr ivr) {
        m_ivr = ivr;
    }
}
