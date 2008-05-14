/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.io.Serializable;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.setting.ProfileNameHandler;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingValue;
import org.sipfoundry.sipxconfig.setting.SettingValueImpl;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class ConferenceBridgeProvisioningImpl extends HibernateDaoSupport implements ConferenceBridgeProvisioning {

    protected static final Log LOG = LogFactory.getLog(ConferenceBridgeProvisioningImpl.class);
    private SipxReplicationContext m_sipxReplicationContext;
    private JobContext m_jobContext;
    private ApiProvider<FreeswitchApi> m_freeswitchApiProvider;

    public void deploy(Serializable bridgeId) {
        Bridge bridge = (Bridge) getHibernateTemplate().load(Bridge.class, bridgeId);
        List conferences = getHibernateTemplate().loadAll(Conference.class);
        generateAdmissionData(bridge, conferences);
        generateConfigurationData(bridge, conferences);
        ApplicationEvent event = new ConferenceConfigReplicatedEvent(this, bridge.getServiceUri());
        m_sipxReplicationContext.publishEvent(event);
        m_sipxReplicationContext.generate(DataSet.ALIAS);
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (!(event instanceof ConferenceConfigReplicatedEvent)) {
            return;
        }
        boolean success = false;
        Serializable jobId = m_jobContext.schedule("FreeSWITCH reload configuration");
        try {
            ConferenceConfigReplicatedEvent e = (ConferenceConfigReplicatedEvent) event;
            m_jobContext.start(jobId);
            FreeswitchApi api = m_freeswitchApiProvider.getApi(e.getServiceUrl());
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

    void generateAdmissionData(Bridge bridge, List conferences) {
        ConferenceAdmission admission = new ConferenceAdmission();
        admission.generate(conferences);
        m_sipxReplicationContext.replicate(admission);
    }

    void generateConfigurationData(Bridge bridge, List conferences) {
        ConferenceConfiguration configuration = new ConferenceConfiguration(bridge);
        configuration.generate(conferences);
        m_sipxReplicationContext.replicate(configuration);
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    @Required
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }
    
    @Required
    public void setFreeswitchApiProvider(ApiProvider<FreeswitchApi> freeswitchApiProvider) {
        m_freeswitchApiProvider = freeswitchApiProvider;
    }

    public static class BostonBridgeFilter implements SettingFilter {
        private static final String PREFIX = "fs-conf";

        public boolean acceptSetting(Setting root_, Setting setting) {
            String profileName = setting.getProfileName();
            return profileName.startsWith(PREFIX);
        }
    }

    public static class ConferenceProfileName implements ProfileNameHandler {
        private static final char SEPARATOR = '.';
        private final String m_conferenceName;

        ConferenceProfileName(String conferenceName) {
            m_conferenceName = SEPARATOR + conferenceName;
        }

        public SettingValue getProfileName(Setting setting) {
            String profileName = setting.getProfileName();
            StringBuffer buffer = new StringBuffer(profileName);
            int dotIndex = profileName.indexOf(SEPARATOR);
            if (dotIndex > 0) {
                buffer.insert(dotIndex, m_conferenceName);
            } else {
                buffer.append(m_conferenceName);
            }

            return new SettingValueImpl(buffer.toString());
        }
    }
}
