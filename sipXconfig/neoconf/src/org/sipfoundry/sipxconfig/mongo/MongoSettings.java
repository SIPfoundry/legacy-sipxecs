/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.mongo;

import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class MongoSettings extends PersistableSettings {

    // difficult to make configurable because system uses this port on initialization
    // and changing would be rebuilding mongo data base
    public static final int SERVER_PORT = 27017;

    // could potentially be changed but not implemented at this time
    public static final int ARBITER_PORT = 27018;

    // could potentially be changed but not implemented at this time
    public static final int LOCAL_PORT = 27019;

    // could potentially be changed but not implemented at this time
    public static final int LOCAL_ARBITER_PORT = 27020;

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("mongo/mongo.xml");
    }

    public int getPort() {
        return (Integer) getSettingTypedValue("mongod/port");
    }

    public boolean disableUseReadTags() {
        return (Boolean) getSettingTypedValue("replication/disableUseReadTags");
    }

    @Override
    public String getBeanId() {
        return "mongoSettings";
    }
}
