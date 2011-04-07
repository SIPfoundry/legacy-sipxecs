/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Arrays;

import org.sipfoundry.sipxconfig.admin.commserver.ConflictingFeatureCodeValidator;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxPresenceService extends SipxService implements LoggingEntity {

    public static final String BEAN_ID = "sipxPresenceService";

    public static final String PRESENCE_SIGN_IN_CODE = "presence-config/SIP_PRESENCE_SIGN_IN_CODE";
    public static final String PRESENCE_SIGN_OUT_CODE = "presence-config/SIP_PRESENCE_SIGN_OUT_CODE";
    public static final String PRESENCE_SERVER_SIP_PORT = "presence-config/PRESENCE_SERVER_SIP_PORT";
    public static final String PRESENCE_API_PORT = "presence-config/SIP_PRESENCE_HTTP_PORT";

    public static final String LOG_SETTING = "presence-config/SIP_PRESENCE_LOG_LEVEL";

    /**
     * Validates the data in this service and throws a UserException if there is a problem
     */
    @Override
    public void validate() {
        SipxService registrarService = getSipxServiceManager().getServiceByBeanId(
                SipxRegistrarService.BEAN_ID);
        new ConflictingFeatureCodeValidator().validate(Arrays.asList(new Setting[] {
            getSettings(), registrarService.getSettings()
        }));
    }

    public int getPresenceServerPort() {
        return (Integer) getSettingTypedValue(PRESENCE_SERVER_SIP_PORT);
    }

    public int getPresenceApiPort() {
        return (Integer) getSettingTypedValue(PRESENCE_API_PORT);
    }

    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void setLogLevel(String logLevel) {
        super.setLogLevel(logLevel);
    }

    @Override
    public String getLogLevel() {
        return super.getLogLevel();
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }
}
