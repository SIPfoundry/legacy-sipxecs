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

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.PatternSettingFilter;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

/**
 * Responsible for generating ipmid.cfg
 */
public class RegAdvancedConfiguration extends ProfileContext {
    private static PatternSettingFilter s_regBasicSettings = new PatternSettingFilter();

    static {
        s_regBasicSettings.addExcludes("reg/address");
        s_regBasicSettings.addExcludes("reg/auth.password");
        s_regBasicSettings.addExcludes("reg/auth.userId");
        s_regBasicSettings.addExcludes("reg/label");
        s_regBasicSettings.addExcludes("reg/outboundProxy.address");
    }

    public RegAdvancedConfiguration(PolycomPhone device) {
        super(device, device.getTemplateDir() + "/reg-advanced.cfg.vm");
    }

    public Collection<Setting> getLines() {
        PolycomPhone phone = (PolycomPhone) getDevice();
        List<Line> lines = phone.getLines();

        // Phones with no configured lines will register under the sipXprovision special user.
        if (lines.isEmpty()) {
            Line line = phone.createSpecialPhoneProvisionUserLine();
            lines.add(line);
        }

        int lineCount = lines.size();

        ArrayList<Setting> linesSettings = new ArrayList<Setting>(lineCount);

        for (Line line : lines) {
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    @Override
    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        Setting endpointSettings = getDevice().getSettings();
        Setting call = endpointSettings.getSetting(PolycomPhone.CALL);
        Collection items = SettingUtil.filter(s_regBasicSettings, call);
        context.put(PolycomPhone.CALL, items);
        context.put("lines", getLines());
        return context;
    }
}
