/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.Setting;

public class AudioCodesGateway extends Gateway {
    private static final String MANUFACTURER = "audiocodes";

    @Override
    public void initialize() {
        AudioCodesGatewayDefaults defaults = new AudioCodesGatewayDefaults(this, getDefaults());
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    protected String getProfileFilename() {
        String serialNumber = getSerialNumber();
        if (serialNumber == null) {
            return null;
        }
        return serialNumber.toUpperCase() + ".ini";
    }

    @Override
    protected ProfileContext createContext() {
        return new AudioCodesContext(this, getProfileTemplate());
    }

    @Override
    protected String getProfileTemplate() {
        AudioCodesModel model = (AudioCodesModel) getModel();
        return model.getProfileTemplate();
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadDynamicModelFile("mp-gateway.xml", MANUFACTURER,
                getSettingsEvaluator());
    }

    @Override
    public Setting loadPortSettings() {
        return getModelFilesContext().loadDynamicModelFile("mp-trunk.xml", MANUFACTURER,
                getSettingsEvaluator());
    }

    @Override
    public void generateProfiles() {
        getProfileGenerator().copy("audiocodes/MP11x-02-1-FXS_16KHZ.dat",
                "MP11x-02-1-FXS_16KHZ.dat");
        getProfileGenerator().copy("audiocodes/usa_tones_12.dat", "usa_tones_12.dat");
        super.generateProfiles();
    }
}
