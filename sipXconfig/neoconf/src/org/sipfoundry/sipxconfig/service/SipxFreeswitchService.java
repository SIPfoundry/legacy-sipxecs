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

import java.io.Serializable;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.conference.FreeswitchApi;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.springframework.beans.factory.annotation.Required;

public class SipxFreeswitchService extends SipxService implements LoggingEntity {
    public static final String FREESWITCH_XMLRPC_PORT = "freeswitch-config/FREESWITCH_XMLRPC_PORT";
    public static final String FREESWITCH_SIP_PORT = "freeswitch-config/FREESWITCH_SIP_PORT";
    public static final String FREESWITCH_MOH_SOURCE = "freeswitch-config/MOH_SOURCE";

    public static final String BEAN_ID = "sipxFreeswitchService";

    public static final String LOG_SETTING = "freeswitch-config/FREESWITCH_SIP_DEBUG";

    public static enum SystemMohSetting {
        FILES_SRC, SOUNDCARD_SRC;

        public static SystemMohSetting parseSetting(String mohSetting) {
            try {
                return valueOf(mohSetting);
            } catch (IllegalArgumentException e) {
                return FILES_SRC;
            }
        }
    }

    private static final String FALSE = "0";
    private static final String TRUE = "1";
    private static final String DEBUG = "DEBUG";
    private static final String INFO = "INFO";
    private static final String NON_DEBUG = "NON-DEBUG";

    private String m_docDir;
    private JobContext m_jobContext;
    private ApiProvider<FreeswitchApi> m_freeswitchApiProvider;
    private SipxProcessContext m_sipxProcessContext;

    public int getXmlRpcPort() {
        return (Integer) getSettingTypedValue(FREESWITCH_XMLRPC_PORT);
    }

    public int getFreeswitchSipPort() {
        return (Integer) getSettingTypedValue(FREESWITCH_SIP_PORT);
    }

    public String getServiceUri(Location location) {
        return String.format("http://%s:%d/RPC2", location.getFqdn(), getXmlRpcPort());
    }

    public boolean reloadXml(Location location) {
        if (!isRunning(location)) {
            // no need to reloadXml if the service is not running at the moment
            return true;
        }
        boolean success = false;
        Serializable jobId = m_jobContext.schedule("FreeSWITCH reload configuration");
        try {
            m_jobContext.start(jobId);
            String serviceUri = getServiceUri(location);
            FreeswitchApi api = m_freeswitchApiProvider.getApi(serviceUri);
            api.reloadxml();
            success = true;
        } finally {
            if (success) {
                m_jobContext.success(jobId);
            } else {
                m_jobContext.failure(jobId, null, null);
            }
        }
        return success;
    }

    boolean isRunning(Location location) {
        return ServiceStatus.Status.Running == m_sipxProcessContext.getStatus(location, this);
    }

    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void setLogLevel(String logLevel) {
        if (logLevel != null && (logLevel.equals(DEBUG) || logLevel.equals(INFO))) {
            super.setLogLevel(TRUE);
        } else {
            super.setLogLevel(FALSE);
        }
    }

    @Override
    public String getLogLevel() {
        String logLevel = super.getLogLevel();
        if (logLevel != null && logLevel.equals(TRUE)) {
            return DEBUG;
        }
        return NON_DEBUG;
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }

    @Required
    public void setDocDir(String docDir) {
        m_docDir = docDir;
    }

    public String getDocDir() {
        return m_docDir;
    }

    @Required
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    @Required
    public void setFreeswitchApiProvider(ApiProvider<FreeswitchApi> freeswitchApiProvider) {
        m_freeswitchApiProvider = freeswitchApiProvider;
    }

    @Required
    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }
}
