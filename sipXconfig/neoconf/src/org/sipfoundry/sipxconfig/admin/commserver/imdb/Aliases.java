/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.common.Replicable;
import static org.sipfoundry.commons.mongo.MongoConstants.ALIASES;

public class Aliases extends DataSetGenerator {
    public static final String FAX_EXTENSION_PREFIX = "~~ff~";

    public Aliases() {
    }

    protected DataSet getType() {
        return DataSet.ALIAS;
    }

    public void generate(Replicable entity, DBObject top) {
        top.put(ALIASES, entity.getAliasMappings(getCoreContext().getDomainName()));
        getDbCollection().save(top);
    }

}
