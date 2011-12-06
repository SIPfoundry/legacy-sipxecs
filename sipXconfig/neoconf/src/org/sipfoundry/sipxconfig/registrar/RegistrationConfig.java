/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Collection;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.springframework.beans.factory.annotation.Required;

public class RegistrationConfig implements ConfigProvider {
    private FeatureManager m_featureManager;
    private Registrar m_registrar;
    private AddressManager m_addressManager;

    @Override
    public void replicate(ConfigManager manager) throws IOException {
        Collection<Location> locations = m_featureManager.getLocationsForEnabledFeature(Registrar.FEATURE);
        RegistrarSettings settings = m_registrar.getSettings();
        String domainName = manager.getDomainManager().getDomainName();
        String realm = manager.getDomainManager().getAuthorizationRealm();
        for (Location location : locations) {
            File f = new File(manager.getLocationDataDirectory(location), "sipxregistrar-config.cfdat");
            FileWriter w = new FileWriter(f);
            KeyValueConfiguration file = new KeyValueConfiguration(w);
            file.write(settings.getSettings());
            Address imXmlRpc = m_addressManager.getSingleAddress(ImManager.XMLRPC_ADDRESS);
            
            String openfireUrl = format("http://%s:%d/plugins/sipx-openfire-presence/status", imXmlRpc.getAddress(),
                    imXmlRpc.getPort());            
            file.write("SIP_REDIRECT.900-PRESENCE.OPENFIRE_PRESENCE_SERVER_URL", openfireUrl);
            
            String presenceMonitorUrl = format("http://%s:%d/RPC2", location.getAddress(), MONITOR_PORT);
            
            file.write("SIP_REDIRECT.900-PRESENCE.LOCAL_PRESENCE_MONITOR_SERVER_URL",
                    presenceMonitorUrl);
            file.write("SIP_REDIRECT.900-PRESENCE.REALM", realm);
            file.write("SIP_REDIRECT.900-PRESENCE.SIP_DOMAIN", domainName);
            w.close();
        }
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Required
    public void setRegistrar(Registrar registrar) {
        m_registrar = registrar;
    }

    @Required
    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }
}
