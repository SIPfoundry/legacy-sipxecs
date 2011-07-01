/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;

public class MongodConfigurationFile extends TemplateConfigurationFile {

    public MongodConfigurationFile() {
        setTemplate("mongo/mongod.vm");
    }
}
