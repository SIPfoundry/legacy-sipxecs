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
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbSettingAdaptor;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.setting.ProfileNameHandler;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.sipfoundry.sipxconfig.setting.SettingValue;
import org.sipfoundry.sipxconfig.setting.SettingValueImpl;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcClientInterceptor;

import org.springframework.aop.framework.ProxyFactory;
import org.springframework.context.ApplicationEvent;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class ConferenceBridgeProvisioningImpl extends HibernateDaoSupport implements
        ConferenceBridgeProvisioning {

    protected static final Log LOG = LogFactory.getLog(ConferenceBridgeProvisioningImpl.class);
    private SipxReplicationContext m_sipxReplicationContext;
    private JobContext m_jobContext;

    public interface FreeSWITCHFunctions {
        void freeswitch_api(String funcion, String params);
    }
    
    public void deploy(Serializable bridgeId) {
        Bridge bridge = (Bridge) getHibernateTemplate().load(Bridge.class, bridgeId);
        List conferences = getHibernateTemplate().loadAll(Conference.class);
        generateAdmissionData(bridge, conferences);
        generateConfigurationData(bridge, conferences);        
        m_sipxReplicationContext.publishEvent(new ConferenceConfigReplicatedEvent(this, bridge.getServiceUri()));
        m_sipxReplicationContext.generate(DataSet.ALIAS);
    }
    
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ConferenceConfigReplicatedEvent) {
            boolean success = false;
            Serializable jobId = m_jobContext.schedule("FreeSWITCH reload configuration");
            try {
                m_jobContext.start(jobId);
                String serviceUrl = ((ConferenceConfigReplicatedEvent) event).getServiceUrl();
                XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
                interceptor.setServiceInterface(FreeSWITCHFunctions.class);
                interceptor.setServiceUrl(serviceUrl);
                interceptor.afterPropertiesSet();

                FreeSWITCHFunctions proxy = (FreeSWITCHFunctions) ProxyFactory.getProxy(FreeSWITCHFunctions.class,
                    interceptor);
                        
                proxy.freeswitch_api("reloadxml", "");
                success = true;
            } finally {
                if (success) {
                    m_jobContext.success(jobId);
                } else {
                    m_jobContext.failure(jobId, null, null);
                }                
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

    void deploy(Bridge bridge, ConfigDbSettingAdaptor adaptor) {
        // TODO - need to remove deleted conferences
        Collection allSettings = new ArrayList();
        Setting settings = bridge.getSettings();
        SettingFilter bbFilter = new BostonBridgeFilter();
        allSettings.addAll(SettingUtil.filter(bbFilter, settings));
        // collect all settings from and push them to adaptor
        Set conferences = bridge.getConferences();
        for (Iterator i = conferences.iterator(); i.hasNext();) {
            Conference conference = (Conference) i.next();
            conference.generateRemoteAdmitSecret();
        }
        adaptor.set("bbridge.conf", allSettings);
        getHibernateTemplate().saveOrUpdateAll(conferences);
    }
 
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }
    
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
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
