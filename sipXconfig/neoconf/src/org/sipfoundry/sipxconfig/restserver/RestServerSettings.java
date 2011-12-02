/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.restserver;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;


public class RestServerSettings extends BeanWithSettings implements DeployConfigOnEdit {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxrest/sipxrest.xml");
    }

    public int getExternalPort() {
        return (Integer) getSettingTypedValue("rest-config/extHttpPort");
    }

    public int getHttpsPort() {
        return (Integer) getSettingTypedValue("rest-config/httpsPort");
    }

    public int getSipPort() {
        return (Integer) getSettingTypedValue("rest-config/sipPort");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) RestServer.FEATURE);
    }
}
