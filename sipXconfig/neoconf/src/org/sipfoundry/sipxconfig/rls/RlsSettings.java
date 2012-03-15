/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.rls;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class RlsSettings extends PersistableSettings implements DeployConfigOnEdit {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxrls/sipxrls.xml");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) Rls.FEATURE);
    }

    public int getTcpPort() {
        return (Integer) getSettingTypedValue("rls-config/SIP_RLS_TCP_PORT");
    }

    public int getUdpPort() {
        return (Integer) getSettingTypedValue("rls-config/SIP_RLS_UDP_PORT");
    }

    @Override
    public String getBeanId() {
        return "rlsSettings";
    }
}
