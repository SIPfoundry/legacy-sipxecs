/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.imbot;

import static org.apache.commons.lang.RandomStringUtils.randomAlphanumeric;
import static org.apache.commons.lang.StringUtils.defaultIfEmpty;
import static org.apache.commons.lang.StringUtils.replace;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ImBotSettings extends BeanWithSettings {
    private static final String PA_USER_NAME_SETTING = "imbot/imId";
    private static final String PA_PASSWORD_SETTING = "imbot/imPassword";
    private static final String HTTP_PORT = "imbot/httpPort";
    private static final String LOCALE_SETTING = "imbot/locale";
    private static final int PASS_LENGTH = 8;

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipximbot/sipximbot.xml");
    }

    public String getPersonalAssistantImId() {
        return getSettingValue(PA_USER_NAME_SETTING);
    }

    public String getPersonalAssistantImPassword() {
        return defaultIfEmpty(replace(getSettingValue(PA_PASSWORD_SETTING), "\\", "\\\\"), getPersonalAssistantImId());
    }

    @Override
    protected void initialize() {
        setSettingValue(PA_PASSWORD_SETTING, randomAlphanumeric(PASS_LENGTH));
    }

    public int getHttpPort() {
        return (Integer) getSettingTypedValue(HTTP_PORT);
    }

    public void setLocale(String localeString) {
        getSettings().getSetting(LOCALE_SETTING).setValue(localeString);
    }
}
