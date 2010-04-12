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

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

public class AudioCodesFxsGateway extends Phone {
    private static final String CALL_PROGRESS_TONES_FILE = "Media_RTP_RTPC/Telephony/CallProgressTonesFilename";
    private static final String FXS_LOOP_CHARACTERISTICS_FILE =
        "Media_RTP_RTPC/Telephony/FXSLoopCharacteristicsFilename";
    private static final String REL_5_4_OR_LATER = "5.4orLater";
    private static final String REL_5_6_OR_LATER = "5.6orLater";
    private static final String REL_6_0_OR_LATER = "6.0orLater";
    private static final String[] COPY_FILES = {CALL_PROGRESS_TONES_FILE, FXS_LOOP_CHARACTERISTICS_FILE};

    public AudioCodesFxsGateway() {
        setDeviceVersion(AudioCodesModel.REL_6_0);
    }

    @Override
    public void setDeviceVersion(DeviceVersion version) {
        super.setDeviceVersion(version);
        DeviceVersion myVersion = getDeviceVersion();

        if (myVersion == AudioCodesModel.REL_5_4) {
            myVersion.addSupportedFeature(REL_5_4_OR_LATER);
        } else if (myVersion == AudioCodesModel.REL_5_6) {
            myVersion.addSupportedFeature(REL_5_4_OR_LATER);
            myVersion.addSupportedFeature(REL_5_6_OR_LATER);
        } else if (myVersion == AudioCodesModel.REL_6_0) {
            myVersion.addSupportedFeature(REL_5_4_OR_LATER);
            myVersion.addSupportedFeature(REL_5_6_OR_LATER);
            myVersion.addSupportedFeature(REL_6_0_OR_LATER);
        }
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
        return new AudioCodesContext(this, getModel().getProfileTemplate());
    }

    @Override
    protected Setting loadSettings() {
        Setting setting = getModelFilesContext().loadDynamicModelFile("gateway.xml",
                getModel().getModelDir(), getSettingsEvaluator());
        String configDir = new File(((AudioCodesFxsModel) getModel()).getConfigDirectory(),
                getModel().getModelDir()).getAbsolutePath();
        GatewayDirectorySetter gatewayDirectorySetter = new GatewayDirectorySetter(configDir);
        setting.acceptVisitor(gatewayDirectorySetter);
        return setting;
    }

    @Override
    protected Setting loadLineSettings() {
        return getModelFilesContext().loadDynamicModelFile("line.xml", getModel().getModelDir(),
                getSettingsEvaluator());
    }

    private void copyGatewayFiles(ProfileLocation location) {
        String name;
        Setting settings = getSettings();
        String sourceDir = new File(((AudioCodesFxsModel) getModel()).getConfigDirectory(),
                getModel().getModelDir()).getAbsolutePath();
        for (String file : COPY_FILES) {
            name = settings.getSetting(file).getValue();
            getProfileGenerator().copy(location, sourceDir, name, name);
        }
    }

    protected void copyFiles(ProfileLocation location) {
        super.copyFiles(location);
        copyGatewayFiles(location);
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

    protected LineInfo getLineInfo(Line line) {
        DeviceDefaults dd = getPhoneContext().getPhoneDefaults();
        return AudioCodesLineDefaults.getLineInfo(line, dd);
    }

    protected void setLineInfo(Line line, LineInfo lineInfo) {
        AudioCodesLineDefaults.setLineInfo(line, lineInfo);
    }
}
