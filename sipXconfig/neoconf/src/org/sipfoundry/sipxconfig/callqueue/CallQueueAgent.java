/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.callqueue;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;
import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.domain.DomainManager;

import org.springframework.beans.factory.annotation.Required;

public class CallQueueAgent extends BeanWithSettings implements Replicable, DeployConfigOnEdit {

    private static final String ALIAS_RELATION = "callqueueagent";
    private static final String PREFIX_CALLQUEUE_AGENT = "~~cqa~";

    private String m_name;
    private String m_extension;
    private String m_description;

    private DomainManager m_domainManager;
    private CallQueueContext m_callQueueContext;

    private CallQueueTiers m_tiers = new CallQueueTiers();

    /* Enabled */
    public boolean isEnabled() {
        return true;
    }
    /* Name */
    public String getName() {
        return m_name;
    }
    public void setName(String name) {
        m_name = name;
    }

    /* Extension */
    public String getExtension() {
        return m_extension;
    }
    public void setExtension(String extension) {
        m_extension = extension;
    }

    /* Description */
    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public DomainManager getDomainManager() {
        return m_domainManager;
    }

    @Required
    public void setCallQueueContext(CallQueueContext callqueuecontext) {
        m_callQueueContext = callqueuecontext;
    }

    public CallQueueContext getCallQueueContext() {
        return m_callQueueContext;
    }

    public CallQueueTiers getTiers() {
        return m_tiers;
    }

    public void setTiers(CallQueueTiers tiers) {
        m_tiers = tiers;
    }

    public String getContactUri() {
        return String.format("sofia/%s/~~cqa~%s@%s",
                m_domainManager.getDomainName(), getExtension(), m_domainManager.getDomainName());
    }

    @Override
    public void initialize() {
        Setting s = getSettings();
        if (null != s) {
            Setting s1 = s.getSetting("call-queue-agent/use-agent-defaults");
            if (null != s1) {
                Boolean b = (Boolean) s1.getTypedValue();
                if (null != b) {
                    if (b) {
                        addDefaultBeanSettingHandler(new Defaults());
                    }
                }
            }
        }
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        List<AliasMapping> mappings = new ArrayList<AliasMapping>();
        if (getExtension() != null) {
            AliasMapping aliasMapping = new AliasMapping(PREFIX_CALLQUEUE_AGENT + getExtension(),
                    SipUri.format(getExtension(), domainName, false), ALIAS_RELATION);
            mappings.add(aliasMapping);
        }
        return mappings;
    }

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> ds = new HashSet<DataSet>();
        ds.add(DataSet.ALIAS);
        return ds;
    }

    @Override
    public String getIdentity(String domain) {
        return SipUri.stripSipPrefix(SipUri.format(null, PREFIX_CALLQUEUE_AGENT + getExtension(), domain));
    }

    @Override
    public boolean isValidUser() {
        return true;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        return Collections.emptyMap();
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxcallqueue/CallQueueAgent.xml");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) CallQueueContext.FEATURE, (Feature) FreeswitchFeature.FEATURE);
    }

    public String getEntityName() {
        return getClass().getSimpleName();
    }

    public void removeFromQueue(Integer callqueueid) {
        CallQueueTiers tiers = getTiers();
        tiers.removeFromQueue(callqueueid);
    }

    public CallQueue getQueueByTier(CallQueueTier callqueuetier) {
        return getCallQueueContext().loadCallQueue(callqueuetier.getCallQueueId());
    }

    public class Defaults {

        @SettingEntry(path = "call-queue-agent/max-no-answer")
        public String getDefaultMaxNoAnswer() {
            return getCallQueueContext().getSettings().getSettingValue("call-queue-agent/max-no-answer");
        }

        @SettingEntry(path = "call-queue-agent/wrap-up-time")
        public String getDefaultWrapUpTime() {
            return getCallQueueContext().getSettings().getSettingValue("call-queue-agent/wrap-up-time");
        }

        @SettingEntry(path = "call-queue-agent/reject-delay-time")
        public String getDefaultRejectDelayTime() {
            return getCallQueueContext().getSettings().getSettingValue("call-queue-agent/reject-delay-time");
        }

        @SettingEntry(path = "call-queue-agent/busy-delay-time")
        public String getDefaultBusyDelayTime() {
            return getCallQueueContext().getSettings().getSettingValue("call-queue-agent/busy-delay-time");
        }

        @SettingEntry(path = "call-queue-agent/no-answer-delay-time")
        public String getDefaultNoAnswerDelayTime() {
            return getCallQueueContext().getSettings().getSettingValue("call-queue-agent/no-answer-delay-time");
        }
    }

    public void copySettingsTo(CallQueueAgent dst) {
        // Copy CallQueueTiers
        CallQueueTiers newTiers = new CallQueueTiers();
        m_tiers.copyTiersTo(newTiers.getTiers());
        dst.setTiers(newTiers);
        // Copy bean settings
        dst.setSettings(getSettings());
    }
}
