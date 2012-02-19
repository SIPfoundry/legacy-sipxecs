/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mwi;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class MwiSettings extends PersistableSettings implements DeployConfigOnEdit {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxstatus/sipxstatus.xml");
    }

    public int getTcp() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_TCP_PORT")).intValue();
    }

    public int getUdpPort() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_UDP_PORT")).intValue();
    }

    public int getHttpApiPort() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_HTTP_PORT")).intValue();
    }

    public int getHttpsApiPort() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_HTTPS_PORT")).intValue();
    }

    @Override
    public String getBeanId() {
        return "mwiSettings";
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) Mwi.FEATURE);
    }
}
