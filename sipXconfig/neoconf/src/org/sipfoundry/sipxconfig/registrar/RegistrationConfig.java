/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.springframework.beans.factory.annotation.Required;

public class RegistrationConfig implements ConfigProvider {
    private FeatureManager m_featureManager;
    private Registrar m_registrar;

    @Override
    public void replicate(ConfigManager manager) throws IOException {
        Collection<Location> locations = m_featureManager.getLocationsForEnabledFeature(Registrar.FEATURE);
        RegistrarSettings settings = m_registrar.getSettings();
        for (Location location : locations) {
            File f = new File(manager.getLocationDataDirectory(location), "sipxregistrar-config.cfdat");
            FileWriter w = new FileWriter(f);
            KeyValueConfiguration file = new KeyValueConfiguration(w);
            file.write(settings.getSettings());
            file.write(settings.getInternalSettings(location));
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
}
