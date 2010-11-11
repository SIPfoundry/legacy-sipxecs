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
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.conference.FreeswitchApi;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

public class SipxFreeswitchService extends SipxService implements LoggingEntity {
    public static final String FREESWITCH_XMLRPC_PORT = "freeswitch-config/FREESWITCH_XMLRPC_PORT";
    public static final String FREESWITCH_SIP_PORT = "freeswitch-config/FREESWITCH_SIP_PORT";
    public static final String FREESWITCH_MOH_SOURCE = "freeswitch-config/MOH_SOURCE";
    public static final String RELOAD_XML_JOB_TITLE = "FreeSWITCH reload configuration";
    public static final String FREESWITCH_CODECS = "freeswitch-config/FREESWITCH_CODECS";
    public static final String BEAN_ID = "sipxFreeswitchService";
    public static final String LOG_SETTING = "freeswitch-config/FREESWITCH_SIP_DEBUG";
    public static final String G729 = "G729";
    public static final String G729_STATUS = "Permitted G.729AB channels";

    public static final Logger LOG = Logger.getLogger("SipxFreeswitchService.class");

    public static enum SystemMohSetting {
        FILES_SRC, SOUNDCARD_SRC, NONE;

        public static SystemMohSetting parseSetting(String mohSetting) {
            try {
                return valueOf(mohSetting);
            } catch (IllegalArgumentException e) {
                return FILES_SRC;
            }
        }
    }

    private static boolean s_codecG729;
    private static final String FALSE = "0";
    private static final String TRUE = "1";
    private static final String DEBUG = "DEBUG";
    private static final String INFO = "INFO";
    private static final String NON_DEBUG = "NON-DEBUG";

    private String m_docDir;
    private JobContext m_jobContext;
    private ApiProvider<FreeswitchApi> m_freeswitchApiProvider;
    private SipxProcessContext m_sipxProcessContext;
    private SipxReplicationContext m_replicationContext;

    public int getXmlRpcPort() {
        return (Integer) getSettingTypedValue(FREESWITCH_XMLRPC_PORT);
    }

    public int getFreeswitchSipPort() {
        return (Integer) getSettingTypedValue(FREESWITCH_SIP_PORT);
    }

    public String getServiceUri(Location location) {
        return String.format("http://%s:%d/RPC2", location.getFqdn(), getXmlRpcPort());
    }

    /**
     * No matter if the service is installed on more than one location,
     * if at least one location has the G729 codec installed, sipXconfig will consider it
     * available on all locations
     */
    @Override
    public void onInit() {
        setCodecG729(isCodecG729Installed());
    }

    /**
     * No matter if the service is installed on more than one location,
     * if at least one location has the G729 codec installed, sipXconfig will consider it
     * available on all locations
     */
    @Override
    public void onRestart() {
        setCodecG729(isCodecG729Installed());
    }

    private boolean isCodecG729Installed() {
        List<Location> locations =  getLocationsManager().getLocationsForService(this);
        String serviceUri = null;
        FreeswitchApi api = null;
        String result = null;
        for (Location location : locations) {
            if (getSipxServiceManager().isServiceInstalled(location.getId(), BEAN_ID)) {
                serviceUri = getServiceUri(location);
                api = m_freeswitchApiProvider.getApi(serviceUri);
                try {
                    result = api.g729_status();
                    if (StringUtils.contains(result, G729_STATUS)) {
                        return true;
                    }
                } catch (XmlRpcRemoteException xrre) {
                    LOG.error(xrre);
                    return false;
                }
            }
        }
        return false;
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    @Override
    public void afterReplication(Location location) {
        if (location != null) {
            reloadXmlWithRetries(location);
        } else {
            Location[] locations = getLocationsManager().getLocations();
            for (Location l : locations) {
                reloadXmlWithRetries(l);
            }
        }
    }

    @Override
    public void onReload() {
        afterReplication(null);
    }

    private void reloadXml(Location location) {
        if (!isRunning(location)) {
            // no need to reloadXml if the service is not running at the moment
            return;
        }
        boolean success = false;
        Serializable jobId = m_jobContext.schedule(RELOAD_XML_JOB_TITLE);
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
    }

    private void reloadXmlWithRetries(Location location) {
        boolean success = false;
        int maxRetries = 3;
        int retry = 0;
        if (!isRunning(location)) {
            // no need to reloadXml if the service is not running at the moment
            return;
        }

        Serializable jobId = m_jobContext.schedule(RELOAD_XML_JOB_TITLE);
        m_jobContext.start(jobId);
        String serviceUri = getServiceUri(location);
        FreeswitchApi api = m_freeswitchApiProvider.getApi(serviceUri);

        while ((retry <= maxRetries) && !success) {
            LOG.debug("reloadXmlWithRetries() while loop retry:" + retry);
            try {
                api.reloadxml();
                success = true;
            } catch (Exception ex) {
                LOG.debug("reloadXmlWithRetries() caught Exception:" + ex);
            } finally {
                if (success) {
                    m_jobContext.success(jobId);
                } else {
                    LOG.debug("reloadXmlWithRetries() failed at retry " + retry);
                    if (retry == maxRetries) { // failed after retry maxRetries so just give up
                        LOG.error("reloadXmlWithRetries() gives up after retry " + retry);
                        m_jobContext.failure(jobId, null, null);
                    } else {
                        retry++;
                        // retry but sleep for a while
                        try {
                            Thread.sleep(2000);
                        } catch (InterruptedException e) {
                            continue;
                        }
                    }
                }
            }
        }

        LOG.debug("reloadXmlWithRetries() return success at retry " + retry);
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

    @Required
    public void setReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_replicationContext = sipxReplicationContext;
    }

    @Override
    public void onConfigChange() {
        m_replicationContext.generate(DataSet.ALIAS);
    }

    @Override
    public String getWarningKey() {
        if (!isCodecG729()) {
            return "freeswitch.warning";
        } else {
            return null;
        }
    }

    public static boolean isCodecG729() {
        return s_codecG729;
    }

    public static void setCodecG729(boolean mCodecG729) {
        s_codecG729 = mCodecG729;
    }

    public static class Defaults {

        @SettingEntry(path = FREESWITCH_CODECS)
        public List<String> getFreeswitchCodecs() {
            ArrayList<String> returnList = new ArrayList<String>();
            returnList.add("G722");
            returnList.add("PCMU@20i");
            returnList.add("PCMA@20i");
            returnList.add("speex");
            returnList.add("L16");
            if (s_codecG729) {
                returnList.add(G729);
            }
            return returnList;
        }
    }
}
