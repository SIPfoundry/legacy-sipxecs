/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.hitachi;

import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;

public class HitachiPhone extends Phone {
    private static final String USER_ID_SETTING = "USER_ACCOUNT/User_ID";
    private static final String DISPLAY_NAME_SETTING = "USER_ACCOUNT/Displayname";
    private static final String PASSWORD_SETTING = "USER_ACCOUNT/User_Password";
    private static final String REGISTRATION_SERVER_SETTING = "SERVER_SETTINGS/1st_Registrar";

    public HitachiPhone() {
    }

    @Override
    public void initializeLine(Line line) {
        HitachiLineDefaults defaults = new HitachiLineDefaults(line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        HitachiPhoneDefaults defaults = new HitachiPhoneDefaults(getPhoneContext()
                .getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
    }

    /**
     * Check loadrun.ini section [3] for a proper format of the file name.
     *
     * %muser.ini means "3 bytes of MAC address + user.ini"
     */
    @Override
    public String getProfileFilename() {
        String serialNumber = getSerialNumber();
        String prefix = serialNumber.substring(6);
        return prefix + "user.ini";
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        info.setDisplayName(line.getSettingValue(DISPLAY_NAME_SETTING));
        info.setUserId(line.getSettingValue(USER_ID_SETTING));
        info.setPassword(line.getSettingValue(PASSWORD_SETTING));
        // phone setting
        info.setRegistrationServer(getSettingValue(REGISTRATION_SERVER_SETTING));
        return info;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        line.setSettingValue(DISPLAY_NAME_SETTING, lineInfo.getDisplayName());
        line.setSettingValue(USER_ID_SETTING, lineInfo.getUserId());
        line.setSettingValue(PASSWORD_SETTING, lineInfo.getPassword());
        // phone setting
        setSettingValue(REGISTRATION_SERVER_SETTING, lineInfo.getRegistrationServer());
    }
}
