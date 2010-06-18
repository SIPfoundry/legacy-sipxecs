/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.ConflictingFeatureCodeValidator;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

public class SipxAccCodeService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxAccCodeService";

    public static final String LOG_SETTING = "acccode/log.level";
    public static final String AUTH_CODE_PREFIX = "authcode/SIP_AUTH_CODE_PREFIX";
    private static final Log LOG = LogFactory.getLog(SipxAccCodeService.class);



    private String m_promptsDir;
    private String m_docDir;
    private String m_authcodeprefix;


    @Required
    public void setPromptsDir(String promptsDirectory) {
        m_promptsDir = promptsDirectory;
    }

    public String getPromptsDir() {
        return m_promptsDir;
    }

    @Required
    public void setDocDir(String docDirectory) {
        m_docDir = docDirectory;
    }

    public String getDocDir() {
        return m_docDir;
    }

/**
 *      * Validates the data in this service and throws a UserException if there is a problem
 **/
    @Override
    public void validate() {
        SipxService registrarService = getSipxServiceManager().getServiceByBeanId(
                SipxRegistrarService.BEAN_ID);
        new ConflictingFeatureCodeValidator().validate(Arrays.asList(new Setting[] {
            getSettings(), registrarService.getSettings()
        }));
        SipxService presenceService = getSipxServiceManager().getServiceByBeanId(SipxPresenceService.BEAN_ID);
        new ConflictingFeatureCodeValidator().validate(Arrays.asList(new Setting[] {
            getSettings(), presenceService.getSettings()
        }));
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

    @Override
    public void onConfigChange() {
        m_authcodeprefix = getSettingValue(AUTH_CODE_PREFIX);
    }

    public String getAuthCodePrefix() {
        return m_authcodeprefix;
    }

    public void setAuthCodePrefix(String authcodeprefix) {
        m_authcodeprefix = authcodeprefix;
        setSettingValue(SipxAccCodeService.AUTH_CODE_PREFIX, authcodeprefix);
    }
}
