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

import java.io.File;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

public abstract class AudioCodesGateway extends Gateway {
    @Override
    public void initialize() {
        AudioCodesGatewayDefaults defaults = new AudioCodesGatewayDefaults(this, getDefaults());
        addDefaultBeanSettingHandler(defaults);
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
        return new AudioCodesContext(this, getModel().getProfileTemplate());
    }

    @Override
    protected Setting loadSettings() {
        Setting setting = getModelFilesContext().loadDynamicModelFile("gateway.xml",
                getModel().getModelDir(), getSettingsEvaluator());
        String configDir = new File(((AudioCodesModel) getModel()).getConfigDirectory(),
                getModel().getModelDir()).getAbsolutePath();
        GatewayDirectorySetter gatewayDirectorySetter = new GatewayDirectorySetter(configDir);
        setting.acceptVisitor(gatewayDirectorySetter);
        return setting;
    }

    private static class GatewayDirectorySetter extends AbstractSettingVisitor {
        private String m_gatewayDirectory;

        public GatewayDirectorySetter(String directory) {
            m_gatewayDirectory = directory;
        }

        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                FileSetting fileType = (FileSetting) type;
                fileType.setDirectory(m_gatewayDirectory);
            }
        }
    }

    abstract int getMaxCalls();
}
