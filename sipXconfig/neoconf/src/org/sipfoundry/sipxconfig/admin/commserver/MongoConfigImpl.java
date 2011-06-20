/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

public class MongoConfigImpl {

    private MongodConfigurationFile m_mongodConfigurationFile;

    public MongodConfigurationFile getMongodConfigurationFile() {
        return m_mongodConfigurationFile;
    }

    public void setMongodConfigurationFile(MongodConfigurationFile mongodConfigurationFile) {
        this.m_mongodConfigurationFile = mongodConfigurationFile;
    }
}
