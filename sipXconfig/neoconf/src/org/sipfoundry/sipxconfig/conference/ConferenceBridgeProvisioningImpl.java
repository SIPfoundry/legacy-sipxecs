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
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbParameter;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbSettingAdaptor;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.setting.ProfileNameHandler;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.sipfoundry.sipxconfig.setting.SettingValue;
import org.sipfoundry.sipxconfig.setting.SettingValueImpl;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class ConferenceBridgeProvisioningImpl extends HibernateDaoSupport implements
        ConferenceBridgeProvisioning {
    private SipxReplicationContext m_sipxReplicationContext;

    public void deploy(Serializable bridgeId) {
        try {
            Bridge bridge = (Bridge) getHibernateTemplate().load(Bridge.class, bridgeId);
            // get settings for bridge and all conferences

            XmlRpcProxyFactoryBean factory = new XmlRpcProxyFactoryBean();
            factory.setServiceInterface(ConfigDbParameter.class);
            factory.setServiceUrl(bridge.getServiceUri());
            factory.afterPropertiesSet();
            ConfigDbParameter configDb = (ConfigDbParameter) factory.getObject();
            ConfigDbSettingAdaptor adaptor = new ConfigDbSettingAdaptor();
            adaptor.setConfigDbParameter(configDb);
            deploy(bridge, adaptor);
            generateAdmissionData();
            m_sipxReplicationContext.generate(DataSet.ALIAS);
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        }
    }

    void generateAdmissionData() {
        List conferences = getHibernateTemplate().loadAll(Conference.class);
        ConferenceAdmission admission = new ConferenceAdmission();
        admission.generate(conferences);
        m_sipxReplicationContext.replicate(admission);
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

    public static class BostonBridgeFilter implements SettingFilter {
        private static final String PREFIX = "BOSTON_BRIDGE";

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
