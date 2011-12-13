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
import java.io.Writer;
import java.util.Collection;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.springframework.beans.factory.annotation.Required;

public class RegistrationConfig implements ConfigProvider {
    private Registrar m_registrar;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Registrar.FEATURE) || !manager.getFeatureManager().isFeatureEnabled(Registrar.FEATURE)) {
            return;
        }

        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(Registrar.FEATURE);
        RegistrarSettings settings = m_registrar.getSettings();
        String domainName = manager.getDomainManager().getDomainName();
        String realm = manager.getDomainManager().getAuthorizationRealm();
        Address imApi = manager.getAddressManager().getSingleAddress(ImManager.XMLRPC_ADDRESS);
        Address presenceApi =  manager.getAddressManager().getSingleAddress(Registrar.PRESENCE_MONITOR_ADDRESS);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer w = new FileWriter(new File(dir, "registrar-config.cfdat"));
            KeyValueConfiguration file = new KeyValueConfiguration(w);
            file.write(settings.getSettings().getSetting("registrar-config"));
            if (imApi != null) {
                String openfireUrl = format("http://%s:%d/plugins/sipx-openfire-presence/status", imApi.getAddress(),
                        imApi.getPort());
                file.write("SIP_REDIRECT.900-PRESENCE.OPENFIRE_PRESENCE_SERVER_URL", openfireUrl);
            }
            file.write("SIP_REDIRECT.900-PRESENCE.LOCAL_PRESENCE_MONITOR_SERVER_URL", presenceApi.toString());
            file.write("SIP_REDIRECT.900-PRESENCE.REALM", realm);
            file.write("SIP_REDIRECT.900-PRESENCE.SIP_DOMAIN", domainName);

            //TODO: Port a lot more settings

            w.close();
        }
    }

    @Required
    public void setRegistrar(Registrar registrar) {
        m_registrar = registrar;
    }
}
