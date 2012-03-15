/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
