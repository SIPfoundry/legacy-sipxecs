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
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.PatternSettingFilter;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

/**
 * Responsible for generating [MAC]-cfg.
 */
public class SipInteropConfiguration extends ProfileContext {
    private static PatternSettingFilter s_callSettings = new PatternSettingFilter();

    static {
        s_callSettings.addExcludes("call/donotdisturb.*$");
        s_callSettings.addExcludes("call/callWaiting.*$");
        s_callSettings.addExcludes("call/shared.*$");
    }

    public SipInteropConfiguration(PolycomPhone device) {
        super(device, device.getTemplateDir() + "/sip-interop.cfg.vm");
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
        Collection items = SettingUtil.filter(s_callSettings, call);
        context.put(PolycomPhone.CALL, items);
        context.put("lines", getLines());
        return context;
    }

}
