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

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class IvrSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String HTTP_PORT = "ivr/ivr.httpPort";

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) Ivr.FEATURE);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxivr/sipxivr.xml");
    }

    public int getHttpPort() {
        return (Integer) getSettingTypedValue(HTTP_PORT);
    }

    @Override
    public String getBeanId() {
        return "ivrSettings";
    }
}
