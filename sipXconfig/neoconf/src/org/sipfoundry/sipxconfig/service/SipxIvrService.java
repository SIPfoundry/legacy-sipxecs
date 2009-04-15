/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.springframework.beans.factory.annotation.Required;

public class SipxIvrService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxIvrService";

    public static final String LOG_SETTING = "ivr/log.level";

    private String m_vxmlDir;
    private String m_mailstoreDir;
    private String m_scriptsDir;
    private String m_docDir;
    private DialPlanContext m_dialPlanContext;

    @Required
    public void setMailstoreDir(String mailstoreDirectory) {
        m_mailstoreDir = mailstoreDirectory;
    }

    public String getMailstoreDir() {
        return m_mailstoreDir;
    }

    @Required
    public void setVxmlDir(String vxmlDirectory) {
        m_vxmlDir = vxmlDirectory;
    }

    public String getVxmlDir() {
        return m_vxmlDir;
    }

    @Required
    public void setScriptsDir(String scriptsDirectory) {
        m_scriptsDir = scriptsDirectory;
    }

    public String getScriptsDir() {
        return m_scriptsDir;
    }

    @Required
    public void setDocDir(String docDirectory) {
        m_docDir = docDirectory;
    }

    public String getDocDir() {
        return m_docDir;
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public String getVoicemailUrl() {
        String voiceMail = m_dialPlanContext.getVoiceMail();
        return SipUri.fix(voiceMail, getDomainName());
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
