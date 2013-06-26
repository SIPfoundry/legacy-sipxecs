/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.callqueue;

import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;
import org.sipfoundry.sipxconfig.setting.Setting;

public class CallQueue extends CallQueueExtension {

    private String m_audioDirectory;

    public String getAudioDirectory() {
        return m_audioDirectory;
    }

    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    /* Set extension handling for CallQueue */
    public void setExtension(String extension) {
        if (getConditions() == null) {
            FreeswitchCondition condition = createCondition();
            condition.addAction(createAction("callcenter", String.format("queue-%s", extension)));
            addCondition(condition);
        }
        for (FreeswitchCondition condition : getConditions()) {
            if (condition.getField().equals(DESTINATION_NUMBER)) {
                condition.setExpression(extension);
            }
        }
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        // TODO: Put some properties for mongo
        props.putAll(super.getMongoProperties(domain));
        return props;
    }

    @Override
    public boolean isEnabled() {
        return true;
    }

    @Override
    public boolean isValidUser() {
        return true;
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(this);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxcallqueue/CallQueue.xml");
    }
}
