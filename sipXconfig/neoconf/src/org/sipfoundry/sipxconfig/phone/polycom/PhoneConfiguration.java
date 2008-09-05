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

import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;

/**
 * Responsible for generating MAC_ADDRESS.d/phone.cfg
 */
public class PhoneConfiguration extends ProfileContext {
    private static final String PHONE_TEMPLATE = PolycomPhone.TEMPLATE_DIR + "/phone.cfg.vm";
    private static final int TEMPLATE_DEFAULT_LINE_COUNT = 6;

    public PhoneConfiguration(Device device) {
        super(device, getPhoneTemplate(device));
    }

    public static String getPhoneTemplate(Device device) {
        return String.format(PHONE_TEMPLATE);
    }

    public Map<String, Object> getContext() {
        Map context = super.getContext();
        context.put("lines", getLines());
        return context;
    }

    public Collection getLines() {
        PolycomPhone phone = (PolycomPhone) getDevice();
        int lineCount = Math.min(phone.getModel().getMaxLineCount(), TEMPLATE_DEFAULT_LINE_COUNT);
        ArrayList linesSettings = new ArrayList(lineCount);

        List<Line> lines = phone.getLines();
        for (Line line : lines) {
            linesSettings.add(line.getSettings());
        }

        // copy in blank lines of all unused lines because we do not use manufacturer template
        // files
        for (int i = lines.size(); i < lineCount; i++) {
            Line line = phone.createLine();
            line.setPhone(phone);
            line.setPosition(i);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }
}
