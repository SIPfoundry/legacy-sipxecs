/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingArray;

class AudioCodesContext extends ProfileContext {
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
        Collection[] portFlatSettings = getPortFlatSettings();
        if (portFlatSettings != null) {
            context.put("portFlatSettings", portFlatSettings);
        }
        return context;
    }

    private String[] getCodecs() {
        BeanWithSettings gateway = getDevice();
        SettingArray codecs = (SettingArray) gateway.getSettings()
                .getSetting("SIP_coders/Codecs");
        List<String> list = new ArrayList<String>(codecs.getSize());
        for (int i = 0; i < codecs.getSize(); i++) {
            String codecName = (String) codecs.getSetting(i, "CoderName").getTypedValue();
            if (StringUtils.isNotBlank(codecName) && !list.contains(codecName)) {
                list.add(codecName);
            }
        }
        return list.toArray(new String[list.size()]);
    }

    private Collection[] getPortFlatSettings() {
        BeanWithSettings device = getDevice();
        if (device instanceof AudioCodesGateway) {
            AudioCodesGateway gateway = (AudioCodesGateway) device;
            List<FxoPort> ports = gateway.getPorts();
            List<Collection<Setting>> flatSettingsList = new ArrayList<Collection<Setting>>();
            for (FxoPort port : ports) {
                SettingsIron iron = new SettingsIron();
                port.getSettings().acceptVisitor(iron);
                flatSettingsList.add(iron.getFlat());
            }
            return flatSettingsList.toArray(new Collection[flatSettingsList.size()]);
        }
        return null;
    }
}
