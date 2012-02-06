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

import java.util.Collection;
import java.util.Collections;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ImBotSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String PA_USER_NAME_SETTING = "imbot/_imbot.paUserName";
    private static final String PA_PASSWORD_SETTING = "imbot/imbot.paPassword";
    private static final String HTTP_PORT = "imbot/imbot.httpport";
    private static final String LOCALE_SETTING = "imbot/imbot.locale";
    private static final String LOG_LEVEL_SETTING = "imbot/log.level";
    private static final int PASS_LENGTH = 8;

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipximbot/sipximbot.xml");
    }

    public String getPersonalAssistantImId() {
        return getSettingValue(PA_USER_NAME_SETTING);
    }

    public void setPersonalAssistantImPassword(String password) {
        setSettingValue(PA_PASSWORD_SETTING, password);
    }

    public String getPersonalAssistantImPassword() {
        return defaultIfEmpty(replace(getSettingValue(PA_PASSWORD_SETTING), "\\", "\\\\"), getPersonalAssistantImId());
    }

    @Override
    protected void initialize() {
        if (StringUtils.isEmpty(getSettingValue(PA_PASSWORD_SETTING))) {
            setSettingValue(PA_PASSWORD_SETTING, randomAlphanumeric(PASS_LENGTH));
        }
    }

    public int getHttpPort() {
        return (Integer) getSettingTypedValue(HTTP_PORT);
    }

    public void setLocale(String localeString) {
        getSettings().getSetting(LOCALE_SETTING).setValue(localeString);
    }

    public String getLocale() {
        return getSettingValue(LOCALE_SETTING);
    }

    public String getLogLevel() {
        return getSettingValue(LOG_LEVEL_SETTING);
    }

    @Override
    public String getBeanId() {
        return "imBotSettings";
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) ImBot.FEATURE);
    }
}
