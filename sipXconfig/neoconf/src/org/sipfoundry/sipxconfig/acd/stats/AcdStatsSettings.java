/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class AcdStatsSettings extends BeanWithSettings {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxacdstats/sipxacdstats.xml");
    }

    public int getAcdStatsPort() {
        return ((Integer) getSettingTypedValue("configagent-config/CONFIG_SERVER_AGENT_PORT")).intValue();
    }
}
