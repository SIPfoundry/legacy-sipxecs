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
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

class AudioCodesContext extends ProfileContext {

    private static final String FIRST_DEFAULTCLID = "applications-advanced-fxo/TrunkGroup_1_FirstPhoneNumber";

    public AudioCodesContext(Device device, String profileTemplate) {
        super(device, getGatewayTemplate(device, profileTemplate));
    }

    public static String getGatewayTemplate(Device device, String profileTemplate) {
        return String.format(profileTemplate, device.getDeviceVersion().getVersionId());
    }

    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        // $$ is used as ignore value
        context.put("ignore", SettingsIron.IGNORE);

        Device gateway = getDevice();

        SettingsIron iron = new SettingsIron();
        gateway.getSettings().acceptVisitor(iron);

        Collection<Setting> flatSettings = iron.getFlat();
        context.put("parameterTables", AudioCodesGateway.PARAMETER_TABLE_SETTINGS);
        context.put("flatSettings", flatSettings);

        Collection[] portFlatSettings = getPortFlatSettings();
        if (portFlatSettings != null) {
            context.put("portFlatSettings", portFlatSettings);
        }
        context.put("allowedIPs", getAllowedIPs());
        context.put("firstDefaultCLID", getFirstDefaultCLID());

        return context;
    }

    private String[] getAllowedIPs() {
        BeanWithSettings gateway = getDevice();
        Setting setting = gateway.getSettings().getSetting("advanced_general/AllowedIPs");
        if (setting != null) {
            return StringUtils.split(setting.getValue());
        }
        return null;
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

    private String getFirstDefaultCLID() {
        BeanWithSettings gateway = getDevice();
        Setting setting = gateway.getSettings().getSetting(FIRST_DEFAULTCLID);
        if (setting != null) {
            return setting.getValue();
        }
        return null;
    }

}
