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

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Setting;

public class AudioCodesFxsGateway extends Phone {
    private static final String MANUFACTURER = "audiocodes";

    public AudioCodesFxsGateway() {
    }

    @Override
    public void initialize() {
        DeviceDefaults dd = getPhoneContext().getPhoneDefaults();
        AudioCodesGatewayDefaults defaults = new AudioCodesGatewayDefaults(this, dd);
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initializeLine(Line line) {
        AudioCodesLineDefaults defaults = new AudioCodesLineDefaults(line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public String getProfileFilename() {
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
    public String getProfileTemplate() {
        AudioCodesFxsModel model = (AudioCodesFxsModel) getModel();
        return model.getProfileTemplate();
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadDynamicModelFile("gateway.xml", MANUFACTURER,
                getSettingsEvaluator());
    }

    @Override
    protected Setting loadLineSettings() {
        return getModelFilesContext().loadDynamicModelFile("line.xml", MANUFACTURER,
                getSettingsEvaluator());
    }

    @Override
    public void generateProfiles(ProfileLocation location) {
        getProfileGenerator().copy(location, "audiocodes/MP11x-02-1-FXS_16KHZ.dat",
                "MP11x-02-1-FXS_16KHZ.dat");
        getProfileGenerator().copy(location, "audiocodes/usa_tones_12.dat", "usa_tones_12.dat");
        super.generateProfiles(location);
    }

    protected LineInfo getLineInfo(Line line) {
        DeviceDefaults dd = getPhoneContext().getPhoneDefaults();
        return AudioCodesLineDefaults.getLineInfo(line, dd);
    }

    protected void setLineInfo(Line line, LineInfo lineInfo) {
        AudioCodesLineDefaults.setLineInfo(line, lineInfo);
    }
}
