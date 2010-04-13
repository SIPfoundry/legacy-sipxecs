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

import org.sipfoundry.sipxconfig.setting.Setting;

public class AudioCodesFxoGateway extends AudioCodesGateway {
    @Override
    public Setting loadPortSettings() {
        return getModelFilesContext().loadDynamicModelFile("fxo-port.xml",
                getModel().getModelDir(), getSettingsEvaluator());
    }

    /**
     * Number of active calls is equal to number of channels.
     */
    int getMaxCalls() {
        return getPorts().size();
    }
}
