/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mail;

import java.io.File;
import java.io.IOException;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;

public class MailConfig implements ConfigProvider {

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(MailManager.FEATURE)) {
            return;
        }

        File gdir = manager.getGlobalDataDirectory();
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(MailManager.FEATURE);
        ConfigUtils.enableCfengineClass(gdir, "mail.cfdat", enabled, "sendmail");
    }
}
