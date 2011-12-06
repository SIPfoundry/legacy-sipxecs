/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rls;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class RlsSettings extends BeanWithSettings implements DeployConfigOnEdit {

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
}
