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

import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;

import static org.sipfoundry.commons.mongo.MongoConstants.USER_LOCATION;

public class UserLocation extends DataSetGenerator {

    @Override
    protected DataSet getType() {
        return DataSet.USER_LOCATION;
    }

    public void generate(Replicable entity, DBObject top) {
        if (entity instanceof User) {
            User user = (User) entity;
            Branch site = user.getSite();
            if (site != null) {
                top.put(USER_LOCATION, site.getName());
            }
            getDbCollection().save(top);
        }
    }

}
