/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Responsible for generating ipmid.cfg
 */
public class SipBasicConfiguration extends ProfileContext {
    private static final String MWI_SUBSCRIBE_SETTING = "msg.mwi/subscribe";
    private static final String BLANK_STRING = "";

    public SipBasicConfiguration(PolycomPhone device) {
        super(device, device.getTemplateDir() + "/sip-basic.cfg.vm");
    }

    public Collection<Setting> getLines() {
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

        int lineCount = lines.size();

        ArrayList<Setting> linesSettings = new ArrayList<Setting>(lineCount);

        for (Line line : lines) {
            if (line.getUser() != null && line.getUser().getSettings() != null
                    && !line.getUser().hasPermission(PermissionName.VOICEMAIL)) {
                line.setSettingValue(MWI_SUBSCRIBE_SETTING, BLANK_STRING);
            }
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    @Override
    public Map<String, Object> getContext() {
        Map context = super.getContext();
        context.put("lines", getLines());
        return context;
    }
}
