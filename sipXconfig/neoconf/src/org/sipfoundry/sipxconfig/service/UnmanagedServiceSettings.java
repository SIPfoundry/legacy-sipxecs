/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class UnmanagedServiceSettings extends PersistableSettings {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("unmanaged/services.xml");
    }

    @Override
    public String getBeanId() {
        return "unmanagedServiceSettings";
    }
}
