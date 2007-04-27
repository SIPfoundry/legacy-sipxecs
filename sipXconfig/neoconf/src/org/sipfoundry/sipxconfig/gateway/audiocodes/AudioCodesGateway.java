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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingArray;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.SettingVisitor;

public abstract class AudioCodesGateway extends Gateway {
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
        return getModelFilesContext().loadDynamicModelFile("mp-gateway.xml", "audiocodes",
                getSettingsEvaluator());
    }

    @Override
    public void generateProfiles() {
        getProfileGenerator().copy("audiocodes/MP11x-02-1-FXS_16KHZ.dat",
                "MP11x-02-1-FXS_16KHZ.dat");
        getProfileGenerator().copy("audiocodes/usa_tones_12.dat", "usa_tones_12.dat");
        super.generateProfiles();
    }

    static class AudioCodesContext extends ProfileContext {
        public AudioCodesContext(BeanWithSettings device, String profileTemplate) {
            super(device, profileTemplate);
        }

        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            // $$ is used as ignore value
            context.put("ignore", "$$");
            context.put("codecs", getCodecs());

            SettingsIron iron = new SettingsIron();
            getDevice().getSettings().acceptVisitor(iron);
            context.put("flatSettings", iron.getFlat());
            return context;
        }

        private String[] getCodecs() {
            BeanWithSettings gateway = getDevice();
            SettingArray codecs = (SettingArray) gateway.getSettings().getSetting(
                    "SIP_coders/Codecs");
            List<String> list = new ArrayList<String>(codecs.getSize());
            for (int i = 0; i < codecs.getSize(); i++) {
                String codecName = (String) codecs.getSetting(i, "CoderName").getTypedValue();
                if (StringUtils.isNotBlank(codecName) && !list.contains(codecName)) {
                    list.add(codecName);
                }
            }
            return list.toArray(new String[list.size()]);
        }

    }

    static class SettingsIron implements SettingVisitor {
        private Collection<Setting> m_flat = new ArrayList<Setting>();

        public Collection<Setting> getFlat() {
            return m_flat;
        }

        public void visitSetting(Setting setting) {
            m_flat.add(setting);
        }

        public boolean visitSettingArray(SettingArray array) {
            // skip arrays
            return false;
        }

        public boolean visitSettingGroup(SettingSet group) {
            // skip empty groups
            if (group.isLeaf()) {
                return false;
            }
            if (group.getParent() != null) {
                visitSetting(group);
            }
            return true;
        }
    }
}
