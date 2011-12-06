/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class CdrSettings extends BeanWithSettings {
    private static final String CDR_REMOTE_ACCESS_IP = "callresolver/SIP_CALLRESOLVER_REMOTE_ACCESS";

    public int getAgentPort() {
        return (Integer) getSettingTypedValue("callresolver/SIP_CALLRESOLVER_AGENT_PORT");
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxcallresolver/sipxcallresolver.xml");
    }
}
