/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.callqueue;

import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;

public class CallQueueSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String BEAN_NAME = "callQueueSettings";
    private static final String SETTING_MAX_NO_ANSWER = "call-queue-agent/max-no-answer";
    private static final String SETTING_WRAP_UP_TIME = "call-queue-agent/wrap-up-time";

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) CallQueueContext.FEATURE, (Feature) FreeswitchFeature.FEATURE);
    }

    @Override
    public String getBeanId() {
        return BEAN_NAME;
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxcallqueue/CallQueueSettings.xml");
    }

    public int getMaxNoAnswer() {
        return (Integer) getSettingTypedValue(SETTING_MAX_NO_ANSWER);
    }

    public int getWrapUpTime() {
        return (Integer) getSettingTypedValue(SETTING_WRAP_UP_TIME);
    }
}
