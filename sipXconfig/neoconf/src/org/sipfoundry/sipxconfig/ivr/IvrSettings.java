/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.ivr;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class IvrSettings extends BeanWithSettings implements DeployConfigOnEdit {
    private static final String HTTPS_PORT = "ivr/httpsPort";
    private static final String NAME_DIAL_PFX = "ivr/nameDialPrefix";
    private static final String DEFAULT_TUI = "ivr/defaultTui";

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) Ivr.FEATURE);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxivr/sipxivr.xml");
    }

    public String getHttpsPort() {
        return getSettingValue(HTTPS_PORT);
    }

    public String getNameDialPrefix() {
        return getSettingValue(NAME_DIAL_PFX);
    }

    public String getDefaultTui() {
        return getSettingValue(DEFAULT_TUI);
    }
}
