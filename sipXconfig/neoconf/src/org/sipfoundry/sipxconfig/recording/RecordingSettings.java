/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.recording;


import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class RecordingSettings extends PersistableSettings implements DeployConfigOnEdit {
    private DomainManager m_domainManager;

    public RecordingSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(path = "recording/recording.sipxchangeDomainName")
        public String getDomainName() {
            return m_domainManager.getDomainName();
        }
        @SettingEntry(path = "recording/recording.realm")
        public String getRealm() {
            return m_domainManager.getAuthorizationRealm();
        }
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxrecording/sipxrecording.xml");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) Recording.FEATURE);
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Override
    public String getBeanId() {
        return "recordingSettings";
    }
}
