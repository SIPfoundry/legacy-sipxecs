/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.ivr;

import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class CallPilotSettings extends PersistableSettings {
    private static final String NAME_DIAL_PFX = "config/nameDialPrefix";
    private static final String DEFAULT_TUI = "config/defaultTui";

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxivr/callpilot.xml");
    }

    public String getNameDialPrefix() {
        return getSettingValue(NAME_DIAL_PFX);
    }

    public String getDefaultTui() {
        return getSettingValue(DEFAULT_TUI);
    }

    @Override
    public String getBeanId() {
        return "callPilotSettings";
    }
}
