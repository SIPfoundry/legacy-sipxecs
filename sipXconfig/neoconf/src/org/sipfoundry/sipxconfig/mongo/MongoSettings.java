/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class MongoSettings extends PersistableSettings {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("mongo/mongo.xml");
    }

    public int getPort() {
        return (Integer) getSettingTypedValue("mongod/port");
    }

    @Override
    public String getBeanId() {
        return "mongoSettings";
    }
}
