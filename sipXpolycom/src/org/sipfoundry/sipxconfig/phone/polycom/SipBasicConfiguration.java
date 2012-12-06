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

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Responsible for generating ipmid.cfg
 */
public class SipBasicConfiguration extends ProfileContext {
    private static final String DEVICE_TEMPLATE = PolycomPhone.TEMPLATE_DIR + "/sip-basic.cfg.vm";
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
            if (line.getUser() != null && !line.getUser().hasPermission(PermissionName.VOICEMAIL)) {
                line.setSettingValue(MWI_SUBSCRIBE_SETTING, BLANK_STRING);
            }
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }
}
