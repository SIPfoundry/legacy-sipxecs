/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rls;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.util.CollectionUtils;

public class RlsConfig implements ConfigProvider, DaoEventListener {
    private static final Collection<Feature> INTERESTED_AREAS = Arrays.asList(new Feature[] {
        Rls.FEATURE
    });
    private boolean m_replicateConfigs;
    private Rls m_rls;

    @Override
    public void replicate(ConfigManager manager, Set<Feature> affectedFeatures) throws IOException {
        if (m_replicateConfigs || CollectionUtils.containsAny(affectedFeatures, INTERESTED_AREAS)) {
            Location location = manager.getFeatureManager().getLocationForEnabledFeature(Rls.FEATURE);
            if (location != null) {
                File file = new File(manager.getLocationDataDirectory(location), "sipxrls-config.cfdat");
                FileWriter wtr = new FileWriter(file);
                RlsSettings settings = m_rls.getSettings();
                KeyValueConfiguration config = new KeyValueConfiguration(wtr);
                config.write(settings.getSettings().getSetting("rls-config"));
                config.write("SIP_RLS_BIND_IP", location.getAddress());
                config.write("SIP_RLS_DOMAIN_NAME", manager.getDomainManager().getDomainName());
                config.write("SIP_RLS_AUTHENTICATE_REALM", manager.getDomainManager().getAuthorizationRealm());
            }
            m_replicateConfigs = false;
        }
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof SpeedDial) {
            m_replicateConfigs = true;
        }
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof SpeedDial) {
            m_replicateConfigs = true;
        }
    }

    @Required
    public void setRls(Rls rls) {
        m_rls = rls;
    }
}
