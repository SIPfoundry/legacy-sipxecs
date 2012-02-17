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

    // difficult to make configurable because system uses this port on initialization
    // and changing would be rebuilding mongo data base
    public static final int SERVER_PORT = 27017;

    // could potentially be changed but not implements at this time
    public static final int ARBITER_PORT = 27018;

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
