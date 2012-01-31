/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class CdrSettings extends PersistableSettings {

    public int getAgentPort() {
        return (Integer) getSettingTypedValue("callresolver/SIP_CALLRESOLVER_AGENT_PORT");
    }

    public int getPostresPort() {
        return 5432; // not configurable
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxcallresolver/sipxcallresolver.xml");
    }

    @Override
    public String getBeanId() {
        return "cdrSettings";
    }
}
