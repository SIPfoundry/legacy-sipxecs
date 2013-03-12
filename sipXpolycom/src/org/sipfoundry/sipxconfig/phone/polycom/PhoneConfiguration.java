/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Responsible for generating MAC_ADDRESS.d/phone.cfg
 */
public class PhoneConfiguration extends ProfileContext {

    // The number of blank lines in polycom_phone1.cfg.
    public static final int TEMPLATE_DEFAULT_LINE_COUNT = 6;

    private static final String MWI_SUBSCRIBE_SETTING = "msg.mwi/subscribe";
    private static final String BLANK_STRING = "";

    public PhoneConfiguration(PolycomPhone device) {
        super(device, device.getTemplateDir() + "/phone.cfg.vm");
    }

    @Override
    public Map<String, Object> getContext() {
        Map context = super.getContext();
        context.put("lines", getLines());
        return context;
    }

    public Collection getLines() {
        PolycomPhone phone = (PolycomPhone) getDevice();
        List<Line> lines = phone.getLines();

        // Phones with no configured lines will register under the sipXprovision special user.
        if (lines.isEmpty()) {
            Line line = phone.createSpecialPhoneProvisionUserLine();
            line.setSettingValue("reg/label", line.getUser().getDisplayName());
            line.setSettingValue(MWI_SUBSCRIBE_SETTING, BLANK_STRING);
            line.setSettingValue("msg.mwi/callBackMode", "disabled");
            lines = new LinkedList<Line>();
            lines.add(line);
        }

        int lineCount = Math.max(lines.size(), TEMPLATE_DEFAULT_LINE_COUNT);
        if (phone.getDeviceVersion() == PolycomModel.VER_4_0_X || phone.getDeviceVersion() == PolycomModel.VER_4_1_X) {
            lineCount = lines.size();
        }

        ArrayList linesSettings = new ArrayList(lineCount);

        for (Line line : lines) {
            if (line.getUser() != null && !line.getUser().hasPermission(PermissionName.VOICEMAIL)) {
                line.setSettingValue(MWI_SUBSCRIBE_SETTING, BLANK_STRING);
            }
            linesSettings.add(line.getSettings());
        }

        // If the device has less than TEMPLATE_DEFAULT_LINE_COUNT lines, then
        // add enough blanks lines (with appropriate sipXecs settings) to
        // override the blank lines in polycom_phone1.cfg.
        for (int i = lines.size(); i < lineCount; i++) {
            Line line = phone.createLine();
            line.setPhone(phone);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    public String[] getLineEmergencySetting(Setting lineset) {
        String emergencyValue = lineset.getSetting("line-dialplan/digitmap/routing.1/emergency.1.value").getValue();

        return StringUtils.split(emergencyValue, ",");
    }
}
