/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.clearone;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class ClearonePhone extends Phone {
    public static final String BEAN_ID = "clearone";
    public static final String CONFIG_FILE = "C1MAXIP_%s.txt";
    public static final String DIALPLAN_FILE = "c1dialplan_%s.txt";

    public ClearonePhone() {
    }

    public String getDialplanTemplate() {
        return "clearone/c1dialplan.txt.vm";
    }

    @Override
    public void initializeLine(Line line) {
        ClearoneLineDefaults defaults = new ClearoneLineDefaults(line, getPhoneContext()
                .getPhoneDefaults());
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        PhoneContext phoneContext = getPhoneContext();
        SpeedDial speedDial = phoneContext.getSpeedDial(this);
        if (speedDial != null) {
            ClearonePhoneSpeedDial phoneSpeedDial = new ClearonePhoneSpeedDial(speedDial);
            addDefaultSettingHandler(phoneSpeedDial);
        }
        ClearonePhoneDefaults defaults = new ClearonePhoneDefaults(phoneContext
                .getPhoneDefaults(), formatName(DIALPLAN_FILE));
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void generateProfiles(ProfileLocation location) {
        super.generateProfiles(location);
        ProfileContext context = new ProfileContext(this, getDialplanTemplate());
        getProfileGenerator().generate(location, context, getProfileFilter(),
                getDialplanFileName());
    }

    @Override
    public void removeProfiles(ProfileLocation location) {
        super.removeProfiles(location);
        location.removeProfile(getDialplanFileName());
    }

    @Override
    public String getProfileFilename() {
        return formatName(CONFIG_FILE);
    }

    public String getDialplanFileName() {
        return formatName(DIALPLAN_FILE);
    }

    private String formatName(String format) {
        // HACK: in some cases this function is called before serial number is assigned, it needs
        // to work with 'null' serial number
        String serialNumber = StringUtils.defaultString(getSerialNumber()).toUpperCase();
        return String.format(format, serialNumber);
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        return ClearoneLineDefaults.getLineInfo(this, line);
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        ClearoneLineDefaults.setLineInfo(this, line, lineInfo);
    }
}
