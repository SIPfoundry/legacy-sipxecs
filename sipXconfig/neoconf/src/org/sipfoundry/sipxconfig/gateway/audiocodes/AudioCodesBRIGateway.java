/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.setting.Setting;

public class AudioCodesBRIGateway extends AudioCodesGateway {
    @Override
    public Setting loadPortSettings() {
        return getModelFilesContext().loadDynamicModelFile("bri_trunk.xml",
                getModel().getModelDir(), getSettingsEvaluator());
    }

    /**
     * Number of active calls is equal to number of channels
     */
    int getMaxCalls() {
        int activeCalls = 0;
        for (FxoPort port : getPorts()) {
            int minChannel = (Integer) port.getSettingTypedValue("Trunk/MinChannel");
            int maxChannel = (Integer) port.getSettingTypedValue("Trunk/MaxChannel");
            activeCalls += (maxChannel - minChannel) + 1;
        }
        return activeCalls;
    }
}
